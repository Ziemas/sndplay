#pragma once
#include "types.h"
#include <fstream>
#include <iostream>
#include <map>
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

struct sbHeader {
    /* 0x0  */ FileType Type;
    /* 0x4  */ u32 version;  // needs to be 3 for seq data to load
    /* 0x8  */ u32 bankOffset;
    /* 0xc  */ u32 sbSize;  // Size of sb+song+instrument+region data
    /* 0x10 */ u32 sampleOffset;
    /* 0x14 */ u32 sampleSize;  // size of sound bank
    /* 0x18 */ u32 seqOffset;   // pointer to sequenced music data
    /* 0x1C */ u32 unused;
};

struct sbv2Struct {
    /* 0x0  */ u32 fourcc;   // "SBv2"
    /* 0x4  */ u32 version;  // maybe
    /* 0x8  */ u32 unkFlags;
    /* 0xC  */ u32 name;             // eg "V1RA" or "BOSS"
    /* 0X10 */ u32 unk;              // 0xa some kind of identifier?
    /* 0x14 */ u16 songCount;        // always 1 for jak banks?
    /* 0x16 */ u16 instrumentCount;  // ?
    /* 0x18 */ u16 regionCount;
    /* 0x1A */ u16 sampleCount;
    /* 0x1C */ u32 songOffset;        // from start of struct
    /* 0x20 */ u32 instrumentOffset;  // these are all rewritten into ptrs
    /* 0x24 */ u32 regionOffset;      //
    /* 0x28 */ u32 spuRamLoc;         // location in spu ram
    /* 0x2C */ u32 sampleSize;        // same as in file header
    /* 0x30 */ u32 nextBank;          // ptr written here
    /* 0x34 */ u32 unk2;
};

struct Song {
    /* 0x0  */ u32 unk;
    /* 0x4  */ u32 bankName;  // these get rewritten into pointers
    /* 0x8  */ u32 midiName;  // in some situations only?
    /* 0xc  */ u32 unkName;   // maybe song name? maybe unused?
    /* 0x10 */ u32 unk2;
    /* 0x14 */ s16 pan;
    /* 0x16 */ u8 midiIdx;
    /* 0x17 */ u8 unk3;
    /* 0x18 */ int midiPtr;  // gets put here
};

struct Instrument {
    u8 nRegion;     // usually 1
    u8 volume;      // ? maybe... like 7f is 1.0?
    u16 something;  // always 0
    u32 oRegion;    // relative to start of SBv2 block I think, points to the
                    // first region for instrument
} __attribute__((packed));

struct Region {    // 0x18 B
    u8 type;       //? always 0
    u8 marker1;    // usually 7f, not always, maybe another volume?
    u8 a;          // not same for same sample data
    u8 b;          // not same for same sample data
    s16 c;         // signed in range +/- 64? Tuning? intruments with 2 regions seem
                   // to have these in a plus minus couple, maybe panorama?
    u32 keymap;    //? LSB is only set for programs with multiple entries?
    u16 marker2;   // 0x80ff, bt sometimes LSB is dX
    u8 type2;      // in range c9 - d1? can be different for same sample data
    u8 marker3;    // always 9f, or 0x80 | 0x1f?
    u16 version;   //? 1 or 0, maybe looped flag? not same sample data
    u32 oSample;   // offset to sample from start of sample bank
    u32 sampleID;  // I think it's the index in the sample bank...
} __attribute__((packed));

struct SoundBank {
    sbv2Struct sbParams;
    std::vector<Instrument> instruments;
    std::map<u32, std::vector<Region>> regions;
    std::vector<u8> samplebuf;

    void Load(sbv2Struct *bank);
};

struct seq {
    u32 version;     // 2
    u32 subversion;  // 1
    u32 number;      // 16
    u32 size;        // Size of sequence data following this struct
} __attribute__((packed));

struct MMID {
    u32 fourcc;    // MMID
    u16 type;      //?
    u8 a;          //?
    u8 nTrack;     // maybe... it's always 3 lol
    char name[4];  // e.g. "V1RA"
    u32 blank;     //? next in list? followed by ptrs to tracks
} __attribute__((packed));

struct MID {
    u32 fourcc;  // MID
    u16 a;
    u16 b;
    u32 c;
    u32 d;
    char name[4];  // e.g. "V1RA"
    u32 e;
    u32 f;  // offset from start of struct to midi data
    u32 g;
    u32 tempo;     // microseconds per quarter note?
    u32 division;  // or not...
    u16 unknown;
    u8 iTrack;  // track index 0 - 2
    u8 nTrack;  // number of tracks (3)
} __attribute__((packed));

struct Tweak {      // TWEAKVAL.MUS, first u32 gives the count of these
    char name[12];  // e.g. "credits"
    u32 tweak;
} __attribute__((packed));
