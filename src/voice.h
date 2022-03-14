// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "bitfield.h"
#include "fifo.h"
#include "types.h"

class voice {
public:
    void run();

private:
    union ADPCMHeader {
        u16 bits;
        bitfield<u16, bool, 10, 1> LoopStart;
        bitfield<u16, bool, 9, 1> LoopRepeat;
        bitfield<u16, bool, 8, 1> LoopEnd;
        bitfield<u16, u8, 4, 3> Filter;
        bitfield<u16, u8, 0, 4> Shift;
    };

    bool m_Noise { false };
    bool m_PitchMod { false };
    bool m_KeyOn { false };
    bool m_KeyOff { false };
    bool m_ENDX { false };

    void DecodeSamples();
    void UpdateBlockHeader();

    fifo<s16, 0x20> m_DecodeBuf {};
    s16 m_DecodeHist1 { 0 };
    s16 m_DecodeHist2 { 0 };
    u32 m_Counter { 0 };

    u16 m_Pitch { 0 };
    s16 m_Out { 0 };

    u16* m_sample { nullptr };
    u32 m_SSA { 0 };
    u32 m_NAX { 0 };
    u32 m_LSA { 0 };
    bool m_CustomLoop { false };

    ADPCMHeader m_CurHeader {};

    // ADSR m_ADSR {};
    // VolumePair m_Volume {};
};