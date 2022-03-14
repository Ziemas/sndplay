// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "types.h"
#include "voice.h"
#include <forward_list>
#include <memory>
#include <unordered_map>

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

class synth {
public:
    synth()
    {
    }


    s16_output tick();
    void key_on(u32 bank, u8 channel, u8 note, u8 velocity);
    void key_off(u32 bank, u8 channel, u8 note, u8 velocity);

private:
    std::forward_list<std::unique_ptr<voice>> m_voices;
    std::unordered_map<u32, std::unique_ptr<u8[]>> m_bank_samples;
};
