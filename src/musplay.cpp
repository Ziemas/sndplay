// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "midi.h"
#include <cassert>
#include <cstdio>
#include <fmt/format.h>
#include <iterator>
#include <memory>

std::vector<soundbank> gBanks;

int loadSongs()
{
    return 0;
}

void soundbank::load_seq()
{
    auto id = (u32*)seqBuf.get();

    fmt::print("seq fourcc {:.4}\n", (char*)id);

    if (*id == FOURCC('M', 'M', 'I', 'D')) {
        auto mmidi = (MultiMIDIBlockHeader*)id;

        // for (int i = 2; i < midi->mmid.nTrack; i++) {
        //     fmt::print("offset {}: {}\n", i, midi->mmid.MID[i]);
        //     MID* m = (MID*)(seqBuf.get() + midi->mmid.MID[i]);
        //     fmt::print("fourcc {:.4}\n", (char*)m);

        //}

        auto midi = (MIDIBlockHeader*)(seqBuf.get() + mmidi->BlockPtr[]);
        midi_player player(midi, sampleBuf.get());
        player.start();
    }

    if (*id == FOURCC('M', 'I', 'D', ' ')) {
        auto midi = (MIDIBlockHeader*)id;

        midi_player player(midi, sampleBuf.get());
        player.start();
    }
}

/*
** sound - song
** prog - instrument
** tone - region
** vag - sample
*/

void soundbank::load(SoundBank* bank)
{
    data = *bank;
    assert(data.DataID == FOURCC('S', 'B', 'v', '2'));

    auto sound = (MIDISound*)((uintptr_t)bank + (uintptr_t)bank->FirstSound);
    for (int i = 0; i < bank->NumSounds; i++) {
        sounds.emplace_back(sound[i]);
    }

    auto tone = (Tone*)((uintptr_t)bank + (uintptr_t)bank->FirstTone);
    for (int i = 0; i < bank->NumTones; i++) {
        tones.emplace_back(tone[i]);
    }

    auto prog = (Prog*)((uintptr_t)bank + (uintptr_t)bank->FirstProg);
    for (int i = 0; i < bank->NumProgs; i++) {
        programs.emplace_back(prog[i]);
    }
}

static int load_bank(char* filename)
{
    std::fstream file;
    file.open(filename, std::fstream::binary | std::fstream::in);

    file.seekg(0, std::fstream::beg);

    if (!file.is_open()) {
        fmt::print("Failed to open file\n");
        return -1;
    }

    sbHeaderData header;
    file.read((char*)&header, sizeof(sbHeaderData));

    // idk if this is what is signified by this at all...
    if (header.Type != FileType::SBv2 && header.Type != FileType::SBlk) {
        fmt::print("{}: Unsupported type: {}\n", filename, (int)header.Type);
        return -1;
    }

    u32 size = header.sbSize;
    if (header.version >= 3) {
        // Don't know why yet
        size += 4;
    }

    auto* soundBank = (SoundBank*)malloc(size);
    if (soundBank == nullptr) {
        fmt::print("malloc failed\n");
        return -1;
    }

    // The following seems pointless because we read into an offset pointer
    // and then never use the base ptr for anything again.
    /*
    u32 *dst = bankBuf;
    if (header.subVer == 3)
    dst = bankBuf + 1;
    file.read((char *)dst, size);
    */

    file.seekg(header.bankOffset, std::fstream::beg);
    file.read((char*)soundBank, header.sbSize);

    soundbank sb;
    sb.load(soundBank);

    if (header.sampleSize == 0) {
        fmt::print("{}: bank size {}\n", filename, header.sampleSize);
        fmt::print("unhandled!\n");
        return 0;
    }

    u8* sampleBuf = (u8*)malloc(header.sampleSize);
    if (soundBank == nullptr) {
        fmt::print("sample malloc failed\n");
        return -1;
    }

    file.seekg(header.sampleOffset, std::fstream::beg);
    sb.sampleBuf = std::make_unique<u8[]>(header.sampleSize);
    file.read((char*)sb.sampleBuf.get(), header.sampleSize);

    seqData sqData;
    file.seekg(header.seqOffset, std::fstream::beg);
    file.read((char*)&sqData, sizeof(seqData));

    sb.seqBuf = std::make_unique<u8[]>(sqData.size);
    file.read((char*)sb.seqBuf.get(), sqData.size);

    sb.load_seq();

    gBanks.push_back(std::move(sb));

    free(sampleBuf);
    free(soundBank);

    file.close();

    return 0;
}
