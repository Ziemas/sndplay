// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "player.h"
#include <fmt/format.h>
#include <fstream>

namespace snd {

player::player()
    : m_synth(m_loader)
{
    cubeb_init(&m_ctx, "OpenGOAL", nullptr);

    cubeb_stream_params outparam = {};
    outparam.channels = 2;
    outparam.format = CUBEB_SAMPLE_S16LE;
    outparam.rate = 48000;
    outparam.layout = CUBEB_LAYOUT_STEREO;
    outparam.prefs = CUBEB_STREAM_PREF_NONE;

    s32 err = 0;
    u32 latency = 0;
    err = cubeb_get_min_latency(m_ctx, &outparam, &latency);
    if (err != CUBEB_OK) {
        throw std::runtime_error("Cubeb failed");
    }

    err = cubeb_stream_init(m_ctx, &m_stream, "OpenGOAL", nullptr, nullptr, nullptr, &outparam, latency, &sound_callback, &state_callback, this);
    if (err != CUBEB_OK) {
        throw std::runtime_error("Cubeb failed");
    }

    err = cubeb_stream_start(m_stream);
    if (err != CUBEB_OK) {
        throw std::runtime_error("Cubeb failed");
    }
}

player::~player()
{
    cubeb_stream_stop(m_stream);
    cubeb_stream_destroy(m_stream);
    cubeb_destroy(m_ctx);
}

long player::sound_callback(cubeb_stream* stream, void* user, const void* input, void* output_buffer, long nframes)
{
    ((player*)user)->tick((s16_output*)output_buffer, nframes);
    return nframes;
}

void player::state_callback(cubeb_stream* stream, void* user, cubeb_state state) {}

void player::tick(s16_output* stream, int samples)
{
    std::scoped_lock lock(m_ticklock);
    static int htick = 200;
    static int stick = 48000;
    for (int i = 0; i < samples; i++) {
        // The handlers expect to tick at 240hz
        // 48000/240 = 200
        if (htick == 200) {
            for (auto it = m_handlers.begin(); it != m_handlers.end();) {
                bool done = it->second->tick();
                if (done) {
                    fmt::print("erasing handler\n");
                    it = m_handlers.erase(it);
                } else {
                    ++it;
                }
            }

            htick = 0;
        }

        if (stick == 48000) {

            fmt::print("{} handlers active\n", m_handlers.size());
            stick = 0;
        }

        stick++;
        htick++;
        *stream++ = m_synth.tick();
    }
}

u32 player::play_midi(MIDISound& sound, s32 vol, s32 pan)
{
    s32 lpan = pan;
    if (pan == -1 || pan == -2) {
        lpan = sound.Pan;
    }

    auto header = (MIDIBlockHeader*)m_loader.get_midi(sound.MIDIID);
    return m_handlers.emplace(std::make_unique<midi_handler>(header, m_synth, (sound.Vol * vol) >> 10, lpan, sound.Repeats, sound.VolGroup, m_loader));
}

u32 player::play_ame(MIDISound& sound, s32 vol, s32 pan)
{
    s32 lpan = pan;
    if (pan == -1 || pan == -2) {
        lpan = sound.Pan;
    }

    auto header = (MultiMIDIBlockHeader*)m_loader.get_midi(sound.MIDIID);
    return m_handlers.emplace(std::make_unique<ame_handler>(header, m_synth, (sound.Vol * vol) >> 10, lpan, sound.Repeats, sound.VolGroup, m_loader));
}

u32 player::play_sound(u32 bank_id, u32 sound_id)
{
    std::scoped_lock lock(m_ticklock);
    try {
        auto& bank = m_loader.get_bank(bank_id);
        auto& sound = bank.sounds.at(sound_id);

        fmt::print("playing sound: {}, type: {}, MIDI: {:.4}\n", sound.Index, sound.Type, (char*)&sound.MIDIID);

        switch (sound.Type) {
        case 4: { // normal MIDI
            return play_midi(sound, 0x400, 0);
        } break;
        case 5: { // AME
            return play_ame(sound, 0x400, 0);
        } break;
        default:
            fmt::print("Unhandled sound type {}\n", sound.Type);
        }

    } catch (std::out_of_range& e) {
        fmt::print("play_sound: requested bank or sound not found\n");
        m_ticklock.unlock();
        return 0;
    }
}

u32 player::load_bank(std::filesystem::path& filepath, size_t offset)
{
    fmt::print("Loading bank {}\n", filepath.c_str());
    std::fstream in(filepath, std::fstream::binary | std::fstream::in);
    in.seekg(offset, std::fstream::beg);

    return m_loader.read_bank(in);
}

}
