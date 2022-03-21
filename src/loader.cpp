// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#define UUID_SYSTEM_GENERATOR
#include "loader.h"
#include "uuid.h"
#include <fmt/format.h>
#include <fstream>

namespace snd {
enum chunk : u32 {
    bank,
    samples,
    midi
};

#define FOURCC(a, b, c, d) \
    ((u32)(((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))

u32 loader::read_music_bank(SoundBankData* data)
{
    fmt::print("Loading music bank {:.4}\n", (char*)&data->BankID);
    uuids::uuid id = uuids::uuid_system_generator{}();
    auto bla = std::hash<uuids::uuid>{};


    return 0;
}

u32 loader::read_sfx_bank(SFXBlockData* data)
{
    return 0;
}

u32 loader::read_bank(std::fstream& in)
{
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
        attr.where[chunk::bank].size += 4;
    }

    auto pos = in.tellg();
    auto bank_buf = std::make_unique<u8[]>(attr.where[chunk::bank].size);
    in.read((char*)bank_buf.get(), attr.where[chunk::bank].size);
    auto bank = (BankTag*)bank_buf.get();

    u32 bank_id = 0;

    if (bank->DataID == FOURCC('S', 'B', 'v', '2')) {
        bank_id = read_music_bank((SoundBankData*)bank_buf.get());
    } else if (bank->DataID == FOURCC('S', 'B', 'l', 'k')) {
        bank_id = read_sfx_bank((SFXBlockData*)bank_buf.get());
    } else {
        throw std::runtime_error("Unknown bank ID, bad file?");
    }

    /*
    in.seekg(attr.where[chunk::bank].offset, std::fstream::beg);

    in.read((char*)&bank.d, sizeof(bank.d));

    in.seekg(attr.where[chunk::bank].offset + bank.d.FirstSound, std::fstream::beg);
    for (int i = 0; i < bank.d.NumSounds; i++) {
        MIDISound sound;
        in.read((char*)&sound, sizeof(sound));
        bank.sounds.emplace_back(sound);
    }

    fmt::print("loaded {} sound(s)\n", bank.sounds.size());

    in.seekg(attr.where[chunk::bank].offset + bank.d.FirstProg, std::fstream::beg);
    for (int i = 0; i < bank.d.NumProgs; i++) {
        Prog prog;
        in.read((char*)&prog.d, sizeof(prog.d));
        bank.programs.emplace_back(std::move(prog));
    }

    for (auto& prog : bank.programs) {
        for (int i = 0; i < prog.d.NumTones; i++) {
            in.seekg(attr.where[chunk::bank].offset + prog.d.FirstTone, std::fstream::beg);
            for (int i = 0; i < prog.d.NumTones; i++) {
                Tone tone;
                in.read((char*)&tone, sizeof(tone));
                tone.BankID = bank.d.BankID;
                // I like to think of SPU ram in terms of shorts, since that's the least addressable unit on it.
                tone.VAGInSR >>= 1;
                prog.tones.emplace_back(tone);
                fmt::print("tone {} vaginsr {:x}\n", i, tone.VAGInSR);
            }
        }
    }

    fmt::print("loaded {} programs and their tones\n", bank.programs.size());

    if (attr.num_chunks >= 2) {
        in.seekg(attr.where[chunk::samples].offset, std::fstream::beg);
        auto samples = std::make_unique<u8[]>(attr.where[chunk::samples].size);
        in.read((char*)samples.get(), attr.where[chunk::samples].size);
        load_samples(bank.d.BankID, std::move(samples));
    }

    fmt::print("vaginsr {:x}\n", bank.d.VagsInSR);

    if (attr.num_chunks >= 3) {
        in.seekg(attr.where[chunk::midi].offset, std::fstream::beg);
        load_midi(in);
    }

    u32 id = bank.d.BankID;
    m_soundbanks.emplace(id, std::move(bank));
    */

    return bank_id;
}

void loader::load_midi(std::fstream& in)
{
    FileAttributes<1> attr;
    u32 cur = in.tellg();

    in.read((char*)&attr, sizeof(attr));
    in.seekg(cur + attr.where[0].offset, std::fstream::beg);

    auto midi = std::make_unique<u8[]>(attr.where[0].size);
    in.read((char*)midi.get(), attr.where[0].size);

    auto h = (MIDIBlock*)midi.get();
    fmt::print("Loaded midi {:.4}\n", (char*)&h->ID);

    m_midi.emplace(h->ID, (MIDIBlock*)midi.get());
    m_midi_chunks.emplace_back(std::move(midi));
}

SoundBank& loader::get_bank(u32 id)
{
    return m_soundbanks.at(id);
}

MIDIBlock* loader::get_midi(u32 id)
{
    return m_midi.at(id);
}

u16* loader::get_bank_samples(u32 id)
{
    return (u16*)m_bank_samples.at(id).get();
}

void loader::load_samples(u32 bank, std::unique_ptr<u8[]> samples)
{
    m_bank_samples.emplace(bank, std::move(samples));
}

}
