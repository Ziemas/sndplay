#include "midi.h"
#include "musplay.h"
#include <fmt/format.h>

/*
** In the original 989snd, the player struct can live in different places
** depending on the type of file.
**
** For files with multiple tracks it lives in-place before the sequence data
** where the file is loaded. For single track (like sunken) it lives separetely
**
*/

/*
** Sequence commands
** 9x xx xx - key on
** Ex xx xx - unk
**
** DX XX - Stop channel / note
**
**
** BX
**
** non-standard commands?
** F0 75 - big function, string of commands until f7?
** F0 F7 - seek to next f7
**
** FF - nop? falls through to common code - was it ff 00?
** FF 2F - loop back to start?
** FF 51 XX XX XX
**
*/

// clang-format off
[[maybe_unused]] static u8 sunken_seq[100] =
{
    0x00, // delta
    0xc0, 0x00, // program change
    0x00,
    0xc1, 0x01,
    0x00,
    0xc2, 0x02,
    0x00,
    0xc3, 0x03,
    0x00,
    0xc4, 0x04,
    0x00,
    0xc5, 0x05,
    0x00,
    0xc6, 0x06,
    0x00,
    0xc7, 0x07,
    0x00,
    0xc8, 0x08,
    0x00,
    0xc9, 0x09,
    0x00,
    0xca, 0x0a,
    0x00,
    0xcb, 0x0b,
    0x00,
    0xcc, 0x0c,
    0x00,
    0xff, 0x51, 0x03, 0x07, 0xa1, 0x20,
    0x00,
    0x96, 0x2d, 0x65,
    0x00,
    0x9c, 0x24, 0x64,
    0x01,
    0x90, 0x35, 0x5d,
    0x00,
    0x94, 0x39, 0x5e,
    0x81, 0x42, // first multibyte delta
    0x9c, 0x24, 0x00, //
    0x0d,
    0x96, 0x2d, 0x00,

    0x84, 0x00, 0x2d, 0x65, 0x00,

    0x9c, 0x24, 0x64, 0x81,
    0x39, 0x96, 0x2d, 0x00,
    0x0c, 0x9c, 0x24, 0x00,
    0x25, 0x90, 0x35, 0x00,
    0x06, 0x91, 0x3e, 0x6f,
    0x81, 0x78, 0x94, 0x39
};
// clang-format on

// player 0x001EC344
// sequence data at 0x001EC3F9
// clang-format off
[[maybe_unused]] static u8 testSeq[56] = {
    0x00, // delta
    0xf0, 0x75, // invoke next level of parsing?
        0x0f, // 0f event
        // reads this chunk of data, into an unknown struct
        0x00, 0x01, 0x00, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x02, 0x01, 0x01, 0x03,
        0x02, 0x02, 0x04, 0x03,
        0x03, 0x05, 0x03, 0x03,
        0x06, 0x03, 0x03, 0x07,
        0x04, 0x04, 0x08, 0x04,
        0x04, 0x09, 0x04, 0x04,
        0x0a, 0x04, 0x04, 0x0b,
        0x04, 0x04, 0x0c, 0x04,
        0x04, 0xf7,

    0x13, 0x00,
    0x00, 0x12, 0x02, 0x11,
    0x01, 0xf7, 0x00, 0x00
};
// clang-format on

std::pair<size_t, u32> midi_player::read_vlq(u8* value)
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

void midi_player::note_on()
{
    fmt::print("{} note on {:02x} {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0], m_seq_ptr[1]);
    m_seq_ptr += 2;
}

void midi_player::note_off()
{
    fmt::print("{} note off {:02x} {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0], m_seq_ptr[1]);
    m_seq_ptr += 2;
}

void midi_player::program_change()
{
    fmt::print("{} program change {:02x} {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0], m_seq_ptr[1]);
    m_seq_ptr += 2;
}

void midi_player::channel_pressure()
{
    fmt::print("{} channel pressure {:02x} {:02x}\n", m_time, m_status, m_seq_ptr[0]);
    m_seq_ptr += 1;
}

void midi_player::channel_pitch()
{
    u8 channel = m_status & 0xF;
    u32 pitch = (m_seq_ptr[0] << 7) | m_seq_ptr[1];
    fmt::print("{}: pitch ch{:01x} {:04x}\n", m_time, channel, pitch);
    m_seq_ptr += 2;
}

void midi_player::meta_event()
{
    fmt::print("{}: meta event {:02x}\n", m_time, *m_seq_ptr);
    size_t len = m_seq_ptr[2];

    if (*m_seq_ptr == 0x2f) {
        fmt::print("End of track!\n");
        // loop point
        // m_seq_ptr = m_seq_data_start;

        m_track_complete = true;
        return;
    }

    for (int i = 0; i < 10; i++) {
        fmt::print("{:x}\n", m_seq_ptr[i]);
    }

    fmt::print("len {:x}\n", m_seq_ptr[2] + 2);
    m_seq_ptr += 2 + len;
}

void midi_player::system_event()
{
    fmt::print("{}: system event {:02x}\n", m_time, *m_seq_ptr);

    switch (*m_seq_ptr) {
    case 0x75:
        while (*m_seq_ptr != 0xf7) {
            m_seq_ptr++;
        }
        m_seq_ptr++;
        break;
    default:
        throw unknown_event();
    }
}

void midi_player::play()
{
    while (!m_track_complete) {
        auto [len, delta] = read_vlq(m_seq_ptr);
        //fmt::print("delta: {} with len {}\n", delta, len);
        m_seq_ptr += len;
        m_time += delta;

        // running status, new events always have top bit
        if (*m_seq_ptr & 0x80) {
            m_status = *m_seq_ptr;
            m_seq_ptr++;
        }

        try {
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
                throw unknown_event();
                return;
            }
        } catch (unknown_event& e) {
            fmt::print("No handler for unknown event {:x}\n", m_status);
            for (int i = 0; i < 10; i++) {
                fmt::print("{:x} ", m_seq_ptr[i]);
            }
            fmt::print("\n");

            break;
        }
    }
}
