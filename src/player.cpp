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

void snd_player::tick(s16_output *stream, int samples) {
    std::scoped_lock lock(m_ticklock);
    for (auto& handler: m_handlers) {
        handler.get()->tick();
    }
}

u32 snd_player::load_bank(std::filesystem::path path)
{
    std::scoped_lock lock(m_ticklock);
    fmt::print("Loading bank {}\n", path.string());
    std::fstream in(path, std::fstream::binary | std::fstream::in);

    FileAttributes attr;
    in.read((char*)(&attr), sizeof(attr));

    fmt::print("type {}\n", attr.type);
    fmt::print("chunks {}\n", attr.num_chunks);

    for (u32 i = 0; i < attr.num_chunks; i++) {
        in.read((char*)&attr.where[i], sizeof(attr.where[i]));

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
            }
        }
    }

    fmt::print("loaded {} programs and their tones\n", bank.programs.size());

    if (attr.num_chunks >= 2) {
        in.seekg(attr.where[1].offset, std::fstream::beg);
        bank.sampleBuf = std::make_unique<u8[]>(attr.where[1].size);
        in.read((char*)bank.sampleBuf.get(), attr.where[1].size);
    }

    if (attr.num_chunks >= 3) {
        in.seekg(attr.where[2].offset, std::fstream::beg);
        auto midi = std::make_unique<u8[]>(attr.where[2].size);
        in.read((char*)midi.get(), attr.where[2].size);

        load_midi(std::move(midi));
    }

    u32 id = bank.d.BankID;
    m_soundbanks.emplace(id, std::move(bank));

    return id;
}

void snd_player::load_midi(std::unique_ptr<u8[]> midi)
{
    std::scoped_lock lock(m_ticklock);
    auto* attr = (FileAttributes*)midi.get();
    u32 id = *(u32*)(midi.get() + attr->where[0].offset);
    fmt::print("midi type {:.4}\n", (char*)&id);
}

void snd_player::play_sound(u32 bank_id, u32 sound_id)
{
    std::scoped_lock lock(m_ticklock);
    try {
        auto& bank = m_soundbanks.at(bank_id);
        auto& sound = bank.sounds.at(sound_id);

        fmt::print("playing sound: {}, type: {}, MIDI: {:.4}\n", sound.Index, sound.Type, (char*)&sound.MIDIID);

        switch (sound.Type) {
        case 4: // normal MIDI
            break;
        case 5: // AME
        default:
            fmt::print("Unhandled sound type {}\n", sound.Type);
        }

    } catch (std::out_of_range& e) {
        fmt::print("play_sound: requested bank or sound not found\n");
        m_ticklock.unlock();
        return;
    }
}
