// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "types.h"
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

// Big thanks to rerwarwar for struct definitions

#define FOURCC(a, b, c, d) \
    ((u32)(((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))

enum class BankFlags : u32 {
    SB_PTRS_REWRITTEN = 0x1,

};

enum class FileType : u32 {
    SBv2 = 1,
    SBlk = 3,
};

// SBlk files have their sbHeader at 0x800 into the file

struct Tone {
    /*   0 */ s8 Priority;
    /*   1 */ s8 Vol;
    /*   2 */ s8 CenterNote;
    /*   3 */ s8 CenterFine;
    /*   4 */ s16 Pan;
    /*   6 */ s8 MapLow;
    /*   7 */ s8 MapHigh;
    /*   8 */ s8 PBLow;
    /*   9 */ s8 PBHigh;
    /*   a */ s16 ADSR1;
    /*   c */ s16 ADSR2;
    /*   e */ s16 Flags;
    /*  10 */ /*void**/u32 VAGInSR;
    /*  14 */ u32 reserved1;
};

struct Prog {
    /*   0 */ s8 NumTones;
    /*   1 */ s8 Vol;
    /*   2 */ s16 Pan;
    /*   4 */ Tone* FirstTone;
};

struct SoundBank;
struct Sound {
    /*   0 */ s32 Type;
    /*   4 */ /*SoundBank**/ u32 Bank;
    /*   8 */ /*void**/ u32 OrigBank;
    /*   c */ s8 OrigType;
    /*   d */ s8 Prog;
    /*   e */ s8 Note;
    /*   f */ s8 Fine;
    /*  10 */ s16 Vol;
    /*  12 */ s8 pad1;
    /*  13 */ s8 VolGroup;
    /*  14 */ s16 Pan;
    /*  16 */ s8 XREFSound;
    /*  17 */ s8 pad2;
    /*  18 */ u16 Flags;
    /*  1a */ u16 pad3;
};

typedef struct MIDISound {
    /*   0 */ s32 Type;
    /*   4 */ /*SoundBank**/ u32 Bank;
    /*   8 */ /*void**/ u32 OrigBank;
    /*   c */ u32 MIDIID;
    /*  10 */ s16 Vol;
    /*  12 */ s8 Repeats;
    /*  13 */ s8 VolGroup;
    /*  14 */ s16 Pan;
    /*  16 */ s8 Index;
    /*  17 */ s8 Flags;
    /*  18 */ /*void**/u32 MIDIBlock;
} MIDISound;

struct SoundBank {
    /*   0 */ u32 DataID;
    /*   4 */ u32 Version;
    /*   8 */ u32 Flags;
    /*   c */ u32 BankID;
    /*  10 */ s8 BankNum;
    /*  11 */ s8 pad1;
    /*  12 */ s16 pad2;
    /*  14 */ s16 NumSounds;
    /*  16 */ s16 NumProgs;
    /*  18 */ s16 NumTones;
    /*  1a */ s16 NumVAGs;
    /*  1c */ /*Sound**/u32 FirstSound;
    /*  20 */ /*Prog**/ u32 FirstProg;
    /*  24 */ /*Tone**/ u32 FirstTone;
    /*  28 */ /*void**/ u32 VagsInSR;
    /*  2c */ u32 VagDataSize;
    /*  30 */ /*SoundBank**/ u32 NextBank;
};

struct MIDIBlockHeader {
    /*   0 */ u32 DataID;
    /*   4 */ s16 Version;
    /*   6 */ s8 Flags;
    /*   7 */ s8 pad1;
    /*   8 */ u32 ID;
    /*   c */ /*void**/ u32 NextMIDIBlock;
    /*  10 */ u32 BankID;
    /*  14 */ /*SoundBank**/ u32 BankPtr;
    /*  18 */ /*s8**/ u32 DataStart;
    /*  1c */ /*s8**/ u32 MultiMIDIParent;
    /*  20 */ u32 Tempo;
    /*  24 */ u32 PPQ;
};

struct MultiMIDIBlockHeader {
    /*   0 */ u32 DataID;
    /*   4 */ s16 Version;
    /*   6 */ s8 Flags;
    /*   7 */ s8 NumMIDIBlocks;
    /*   8 */ u32 ID;
    /*   c */ /*void**/u32 NextMIDIBlock;
    /*  10 */ /*s8**/u32 BlockPtr[1];
};

struct sbHeaderData {
    /* 0x0  */ FileType Type;
    /* 0x4  */ u32 version; // needs to be 3 for seq data to load
    /* 0x8  */ u32 bankOffset;
    /* 0xc  */ u32 sbSize; // Size of sb+song+instrument+region data
    /* 0x10 */ u32 sampleOffset;
    /* 0x14 */ u32 sampleSize; // size of sound bank
    /* 0x18 */ u32 seqOffset; // pointer to sequenced music data
    /* 0x1C */ u32 unused;
};

struct seqData {
    /* 0x0 */ u32 version;     // 2
    /* 0x4 */ u32 subversion;  // 1
    /* 0x8 */ u32 number;      // 16
    /* 0xC */ u32 size;        // Size of sequence data following this struct
};


//struct Instrument {
//    instrumentData data;
//    std::vector<regionData> regions;
//};

//struct Song {
//    songData data;
//    std::vector<MID*> tracks;
//};

struct soundbank {
    SoundBank data;
    std::vector<Tone> tones;
    std::vector<Prog> programs;
    std::vector<MIDISound> sounds;
    std::unique_ptr<u8[]> sampleBuf;
    std::unique_ptr<u8[]> seqBuf;

    void load(SoundBank* bank);
    void load_seq();
};

struct Tweak { // TWEAKVAL.MUS, first u32 gives the count of these
    char name[12]; // e.g. "credits"
    u32 tweak;
};
