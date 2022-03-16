// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "types.h"
#include "voice.h"
#include <forward_list>
#include <memory>
#include <unordered_map>

enum class toneflag : u16 {
    out_reverb = 1,
    out_dry = 0x10,
};

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
    /*  10 */ /*void**/ u32 VAGInSR;
    /*  14 */ u32 reserved1;
};

struct ProgData {
    /*   0 */ s8 NumTones;
    /*   1 */ s8 Vol;
    /*   2 */ s16 Pan;
    /*   4 */ /*Tone**/ u32 FirstTone;
};

struct basic_data {
    /*   0 */ u32 pad1;
};

struct midi_data {
    /*   0 */ s8 MidiChannel;
    /*   1 */ u8 KeyOnVelocity;
    /*   2 */ s16 pad2;
    /*   4 */ s32 ShouldBeOff;
    /*   8 */ u32 KeyOnProg;
};

struct block_data {
    /*   0 */ s8 g_vol;
    /*   1 */ s8 pad;
    /*   2 */ s16 g_pan;
};

union ownerdata_tag {
    /*   0 */ basic_data BasicData;
    /*   0 */ midi_data MIDIData;
    /*   0 */ block_data BlockData;
};

struct SpuVolume {
    /*   0 */ s16 left;
    /*   2 */ s16 right;
};

struct VoiceAttributes {
    /*   0 */ u32 playlist;
    /*   4 */ s32 voice;
    /*   8 */ u32 Status;
    /*   c */ u32 Owner;
    /*  10 */ u32 OwnerProc;
    /*  14 */ u32 Tone;
    /*  18 */ s8 StartNote;
    /*  19 */ s8 StartFine;
    /*  1a */ u8 Priority;
    /*  1b */ s8 VolGroup;
    /*  1c */ u32 StartTick;
    /*  20 */ SpuVolume Volume;
    /*  24 */ s16 Current_PB;
    /*  26 */ s16 Current_PM;
    /*  28 */ u32 Flags;
    /*  2c */ ownerdata_tag OwnerData;
};

struct Prog {
    ProgData d;
    std::vector<Tone> tones;
};

class synth {
public:
    synth()
    {
    }

    s16_output tick();
    void key_on(Tone& tone, u8 channel, u8 note, vol_pair volume);
    void key_off(u8 channel, u8 note, u8 velocity);

    void load_samples(u32 bank, std::unique_ptr<u8[]>);

private:
    std::unique_ptr<u8[]> m_tmp_samples;
    // std::unordered_map<u32, std::unique_ptr<u8[]>> m_bank_samples;
    std::forward_list<std::unique_ptr<voice>> m_voices;
};
