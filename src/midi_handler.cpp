// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "midi_handler.h"
#include "ame_handler.h"
#include "util.h"
#include <SDL.h>
#include <fmt/format.h>
#include <pthread.h>

/*
** In the original 989snd, the player struct can live in different places
** depending on the type of file.
**
** For files with multiple tracks it lives in-place before the sequence data
** where the file is loaded. For single track (like sunken) it lives separetely
**
** the sequencer ticks at 240hz
**
*/

std::pair<size_t, u32> midi_handler::read_vlq(u8* value)
{
    size_t len = 1;
    u32 out = *value & 0x7f;
    // fmt::print("starting with {:x}\n", *value);

    if ((*value & 0x80) != 0) {
        while ((*value & 0x80) != 0) {
            len++;
            value++;
            out = (out << 7) + (*value & 0x7f);
        }
    }

    return { len, out };
}

void midi_handler::mute_channel(u8 channel)
{
    fmt::print("{:x} ame muting channel {}\n", (u64)this, channel);
    m_mute_state[channel] = true;
}

void midi_handler::unmute_channel(u8 channel)
{
    fmt::print("{:x} ame unmuting channel {}\n", (u64)this, channel);
    m_mute_state[channel] = false;
}

void midi_handler::note_on()
{
    u8 channel = m_status & 0xf;
    u8 note = m_seq_ptr[0];
    u8 velocity = m_seq_ptr[1];

    if (velocity == 0) {
        note_off();
        return;
    }

    if (m_mute_state[channel]) {
        m_seq_ptr += 2;
        return;
    }

    fmt::print("{:x} {}: [ch{:01x}] note on {:02x} {:02x}\n", (u64)this, m_time, channel, note, velocity);

    // Key on all the applicable tones for the program
    auto& bank = m_locator.get_bank(m_header->BankID);
    auto& program = bank.programs[m_programs[channel]];

    for (auto& t : program.tones) {
        if (note >= t.MapLow && note <= t.MapHigh) {

            // TODO passing m_pan here makes stuff sound bad, why?
            auto volume = make_volume(m_vol, velocity, 0, program.d.Vol, program.d.Pan, t.Vol, t.Pan);

            m_synth.key_on(t, channel, note, volume, (u64)this);
        }
    }

    m_seq_ptr += 2;
}

void midi_handler::note_off()
{
    u8 channel = m_status & 0xf;
    u8 note = m_seq_ptr[0];
    // Yep, no velocity for note-offs
    [[maybe_unused]] u8 velocity = m_seq_ptr[1];

     fmt::print("{}: note off {:02x} {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0], m_seq_ptr[1]);

    // TODO we need tracking for who owns the voices
    m_synth.key_off(channel, note, (u64)this);
    m_seq_ptr += 2;
}

void midi_handler::program_change()
{
    u8 channel = m_status & 0xf;
    u8 program = m_seq_ptr[0];

    m_programs[channel] = program;

    //fmt::print("{:x} {}: [ch{:01x}] program change {:02x} -> {:02x}\n", (u64)this, m_time, channel, m_programs[channel], program);
    m_seq_ptr += 1;
}

void midi_handler::channel_pressure()
{
    u8 channel = m_status & 0xf;
    u8 note = m_seq_ptr[0];
    // fmt::print("{}: channel pressure {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0]);
    //  TODO we need tracking for who owns the voices
    m_synth.key_off(channel, note, (u64)this);
    m_seq_ptr += 1;
}

void midi_handler::channel_pitch()
{
    u8 channel = m_status & 0xF;
    u32 pitch = (m_seq_ptr[0] << 7) | m_seq_ptr[1];
    fmt::print("{}: pitch ch{:01x} {:04x}\n", m_time, channel, pitch);
    m_seq_ptr += 2;
}

void midi_handler::meta_event()
{
    fmt::print("{}: meta event {:02x}\n", m_time, *m_seq_ptr);
    size_t len = m_seq_ptr[1];

    if (*m_seq_ptr == 0x2f) {
        m_seq_ptr = m_seq_data_start;
        m_repeats--;

        if (m_repeats <= 0) {
            fmt::print("End of track, no more repeats!\n");
            m_track_complete = true;
        } else {
            fmt::print("End of track, repeating!\n");
        }

        return;
    }

    if (*m_seq_ptr == 0x51) {
        m_tempo = (m_seq_ptr[2] << 16) | (m_seq_ptr[3] << 8) | (m_seq_ptr[4]);
    }

    m_seq_ptr += len + 2;
}

void midi_handler::system_event()
{
    // fmt::print("{}: system event {:02x}\n", m_time, *m_seq_ptr);

    switch (*m_seq_ptr) {
    case 0x75:
        m_seq_ptr++;
        if (m_parent.has_value()) {
            auto [cont, ptr] = m_parent.value()->run_ame(*this, m_seq_ptr);
            m_seq_ptr = ptr;

            if (!cont) {
                fmt::print("track stopped by ame\n");
                m_track_complete = true;
            }
        } else {
            throw midi_error("MIDI tried to run AME without an AME handler");
        }
        break;
    default:
        throw midi_error(fmt::format("Unknown system message {:02x}", *m_seq_ptr));
        // while (*m_seq_ptr != 0xf7) {
        //     m_seq_ptr++;
        // }
        // m_seq_ptr++;
    }
}

bool midi_handler::tick()
{
    try {
        step();
    } catch (midi_error& e) {
        m_track_complete = true;
        fmt::print("MIDI Error: {}\n", e.what());

        fmt::print("Sequence following: ");
        for (int i = 0; i < 10; i++) {
            fmt::print("{:x} ", m_seq_ptr[i]);
        }
        fmt::print("\n");
    }

    return m_track_complete;
}

void midi_handler::new_delta(bool reset)
{
    auto [len, delta] = read_vlq(m_seq_ptr);

    m_seq_ptr += len;
    m_time += delta;

    auto delta_mics = m_tempo * delta / m_ppq;
    m_tick_countdown = delta_mics / mics_per_tick;
}

void midi_handler::step()
{
    if (m_get_delta) {
        new_delta(true);
        m_get_delta = false;
    } else {
        m_tick_countdown--;
    }

    while (!m_tick_countdown && !m_track_complete) {
        // running status, new events always have top bit
        if (*m_seq_ptr & 0x80) {

            //fmt::print("\n");
            //for (int i = -2; i < 10; i++) {
            //    fmt::print("{:02x} ", m_seq_ptr[i]);
            //}
            //fmt::print("\n");

            m_status = *m_seq_ptr;
            m_seq_ptr++;
        }

        switch (m_status >> 4) {
        case 0x8:
            note_off();
            break;
        case 0x9:
            note_on();
            break;
        case 0xD:
            channel_pressure();
            break;
        case 0xC:
            program_change();
            break;
        case 0xE:
            channel_pitch();
            break;
        case 0xF:
            // normal meta-event
            if (m_status == 0xFF) {
                meta_event();
                break;
            }
            if (m_status == 0xF0) {
                system_event();
                break;
            }
        default:
            throw midi_error(fmt::format("MIDI error: invalid status {}", m_status));
            return;
        }

        new_delta(false);
    }
}
