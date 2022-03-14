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

// SBlk files have their sbHeader at 0x800 into the file
struct SoundBank;

// Generic
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

// For midi banks


struct seqData {
    /* 0x0 */ u32 version; // 2
    /* 0x4 */ u32 subversion; // 1
    /* 0x8 */ u32 number; // 16
    /* 0xC */ u32 size; // Size of sequence data following this struct
};

// struct Instrument {
//     instrumentData data;
//     std::vector<regionData> regions;
// };

// struct Song {
//     songData data;
//     std::vector<MID*> tracks;
// };

struct Tweak { // TWEAKVAL.MUS, first u32 gives the count of these
    char name[12]; // e.g. "credits"
    u32 tweak;
};


enum class FileType : u32 {
    SBv2 = 1,
    SBlk = 3,
};
