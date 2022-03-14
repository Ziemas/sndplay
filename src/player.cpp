#include "player.h"
#include <fmt/format.h>
#include <fstream>

u32 snd_player::load_bank(std::filesystem::path path)
{
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
    auto* attr = (FileAttributes*)midi.get();
    u32 id = *(u32*)(midi.get() + attr->where[0].offset);
    fmt::print("midi type {:.4}\n", (char*)&id);
}

void snd_player::play_sound(u32 bank_id, u32 sound_id)
{
    try {
        auto& bank = m_soundbanks.at(bank_id);
        auto& sound = bank.sounds.at(sound_id);


        fmt::print("playing sound: {}, type: {}, MIDI: {:.4}\n", sound.Index, sound.Type, (char*)&sound.MIDIID);

        switch (sound.Type) {
            default:
                fmt::print("Unhandled sound type {}\n", sound.Type);
        }

    } catch (std::out_of_range& e) {
        fmt::print("play_sound: requested bank or sound not found\n");
        return;
    }
}
