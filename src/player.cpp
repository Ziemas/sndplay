// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "player.h"
#include <fmt/format.h>
#include <fstream>

namespace snd {

player::player() : m_synth(m_loader)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error("SDL init failed");
    }

    SDL_AudioSpec want {}, got {};
    want.channels = 2;
    want.format = AUDIO_S16;
    want.freq = 48000;
    want.samples = 4096;
    want.callback = &sdl_callback;
    want.userdata = this;

    m_dev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);
    if (m_dev == 0) {
        throw std::runtime_error("SDL OpenAudioDevice failed");
    }

    SDL_PauseAudioDevice(m_dev, 0);
}

player::~player()
{
    SDL_PauseAudioDevice(m_dev, 1);
    SDL_CloseAudioDevice(m_dev);
}

void player::sdl_callback(void* userdata, u8* stream, int len)
{
    int channels = 2;
    int sample_size = 2;
    int samples = (len / sample_size) / channels;
    ((player*)userdata)->tick((s16_output*)stream, samples);
}

void player::tick(s16_output* stream, int samples)
{
    std::scoped_lock lock(m_ticklock);
    static int htick = 200;
    for (int i = 0; i < samples; i++) {
        // The handlers expect to tick at 240hz
        // 48000/240 = 200
        if (htick == 200) {
            for (auto& handler : m_handlers) {
                /*bool done = */ handler.get()->tick();

                // clean up handlers here
                // if (done) {
                //    m_handlers.remove(handler);
                //}
            }

            htick = 0;
        }
        htick++;

        *stream++ = m_synth.tick();
    }
}

void player::play_midi(MIDISound& sound, s32 vol, s32 pan)
{
    auto header = (MIDIBlockHeader*)m_loader.get_midi(sound.MIDIID);
    m_handlers.emplace_front(std::make_unique<midi_handler>(header, m_synth, (sound.Vol * vol) >> 10, sound.Pan, sound.Repeats, sound.VolGroup, m_loader));
}

void player::play_ame(MIDISound& sound, s32 vol, s32 pan)
{
    auto header = (MultiMIDIBlockHeader*)m_loader.get_midi(sound.MIDIID);
    m_handlers.emplace_front(std::make_unique<ame_handler>(header, m_synth, (sound.Vol * vol) >> 10, sound.Pan, sound.Repeats, sound.VolGroup, m_loader));
}

void player::play_sound(u32 bank_id, u32 sound_id)
{
    std::scoped_lock lock(m_ticklock);
    try {
        auto& bank = m_loader.get_bank(bank_id);
        auto& sound = bank.sounds.at(sound_id);

        fmt::print("playing sound: {}, type: {}, MIDI: {:.4}\n", sound.Index, sound.Type, (char*)&sound.MIDIID);

        switch (sound.Type) {
        case 4: { // normal MIDI
            play_midi(sound, 0x400, 0);
        } break;
        case 5: { // AME
            play_ame(sound, 0x400, 0);
        } break;
        default:
            fmt::print("Unhandled sound type {}\n", sound.Type);
        }

    } catch (std::out_of_range& e) {
        fmt::print("play_sound: requested bank or sound not found\n");
        m_ticklock.unlock();
        return;
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
