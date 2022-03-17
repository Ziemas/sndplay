// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "player.h"
#include <fmt/format.h>
#include <fstream>

snd_player::snd_player()
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

snd_player::~snd_player()
{
    SDL_PauseAudioDevice(m_dev, 1);
    SDL_CloseAudioDevice(m_dev);
}

void snd_player::sdl_callback(void* userdata, u8* stream, int len)
{
    // TODO can remove later
    memset(stream, 0, len);

    int channels = 2;
    int sample_size = 2;
    int samples = (len / sample_size) / channels;
    ((snd_player*)userdata)->tick((s16_output*)stream, samples);
}

void snd_player::tick(s16_output* stream, int samples)
{
    std::scoped_lock lock(m_ticklock);
    static int htick = 200;
    for (int i = 0; i < samples; i++) {
        // The handlers expect to tick at 240hz
        // 48000/240 = 200
        if (htick == 200) {
            for (auto& handler : m_handlers) {
                bool done = handler.get()->tick();

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

u32 snd_player::load_bank(std::filesystem::path path)
{
    std::scoped_lock lock(m_ticklock);
    fmt::print("Loading bank {}\n", path.string());
    std::fstream in(path, std::fstream::binary | std::fstream::in);

    FileAttributes<3> attr;
    in.read((char*)(&attr), sizeof(attr));

    fmt::print("type {}\n", attr.type);
    fmt::print("chunks {}\n", attr.num_chunks);

    for (u32 i = 0; i < attr.num_chunks; i++) {
        // in.read((char*)&attr.where[i], sizeof(attr.where[i]));

        fmt::print("chunk {}\n", i);
        fmt::print("\toffset {}\n", attr.where[i].offset);
        fmt::print("\tsize {}\n", attr.where[i].size);
    }

    if (attr.type != 1 && attr.type != 3) {
        fmt::print("Error: File type {} not supported.", attr.type);
        return -1;
    }

    if (attr.num_chunks > 2) {
        // Fix for bugged tooling I assume?
        attr.where[0].size += 4;
    }

    SoundBank bank;

    in.seekg(attr.where[0].offset, std::fstream::beg);
    in.read((char*)&bank.d, sizeof(bank.d));
    fmt::print("{:.4}\n", (char*)&bank.d.BankID);

    in.seekg(attr.where[0].offset + bank.d.FirstSound, std::fstream::beg);
    for (int i = 0; i < bank.d.NumSounds; i++) {
        MIDISound sound;
        in.read((char*)&sound, sizeof(sound));
        bank.sounds.emplace_back(sound);
    }

    fmt::print("loaded {} sound(s)\n", bank.sounds.size());

    in.seekg(attr.where[0].offset + bank.d.FirstProg, std::fstream::beg);
    for (int i = 0; i < bank.d.NumProgs; i++) {
        Prog prog;
        in.read((char*)&prog.d, sizeof(prog.d));
        bank.programs.emplace_back(std::move(prog));
    }

    for (auto& prog : bank.programs) {
        for (int i = 0; i < prog.d.NumTones; i++) {
            in.seekg(attr.where[0].offset + prog.d.FirstTone, std::fstream::beg);
            for (int i = 0; i < prog.d.NumTones; i++) {
                Tone tone;
                in.read((char*)&tone, sizeof(tone));
                prog.tones.emplace_back(tone);
                fmt::print("tone {} vaginsr {:x}\n", i, tone.VAGInSR);
            }
        }
    }

    fmt::print("loaded {} programs and their tones\n", bank.programs.size());

    if (attr.num_chunks >= 2) {
        in.seekg(attr.where[1].offset, std::fstream::beg);
        auto samples = std::make_unique<u8[]>(attr.where[1].size);
        in.read((char*)samples.get(), attr.where[1].size);
        m_synth.load_samples(bank.d.BankID, std::move(samples));
    }

    fmt::print("vaginsr {:x}\n", bank.d.VagsInSR);

    if (attr.num_chunks >= 3) {
        in.seekg(attr.where[2].offset, std::fstream::beg);
        load_midi(in);
    }

    u32 id = bank.d.BankID;
    m_soundbanks.emplace(id, std::move(bank));

    return id;
}

void snd_player::load_midi(std::fstream& in)
{
    std::scoped_lock lock(m_ticklock);
    FileAttributes<1> attr;
    u32 cur = in.tellg();

    in.read((char*)&attr, sizeof(attr));
    in.seekg(cur + attr.where[0].offset, std::fstream::beg);

    auto midi = std::make_unique<u8[]>(attr.where[0].size);
    in.read((char*)midi.get(), attr.where[0].size);

    auto h = (MIDIBlockHeader*)midi.get();
    fmt::print("Loaded midi {:.4}\n", (char*)&h->ID);

    m_midi.emplace(h->ID, (MIDIBlockHeader*)midi.get());
    m_midi_chunks.emplace_back(std::move(midi));
}

void snd_player::play_midi(MIDISound& sound, s32 vol, s32 pan)
{
    auto header = m_midi.at(sound.MIDIID);
    m_handlers.emplace_front(std::make_unique<midi_handler>(header, m_synth, (sound.Vol * vol) >> 10, sound.Pan, sound.Repeats, sound.VolGroup, *this));
}

void snd_player::play_ame(MIDISound& sound, s32 vol, s32 pan)
{
    auto header = (MultiMIDIBlockHeader*)m_midi.at(sound.MIDIID);
    m_handlers.emplace_front(std::make_unique<ame_handler>(header, m_synth, (sound.Vol * vol) >> 10, sound.Pan, sound.Repeats, sound.VolGroup, *this));
}

void snd_player::play_sound(u32 bank_id, u32 sound_id)
{
    std::scoped_lock lock(m_ticklock);
    try {
        auto& bank = m_soundbanks.at(bank_id);
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

SoundBank& snd_player::get_bank(u32 id)
{
    return m_soundbanks.at(id);
}
