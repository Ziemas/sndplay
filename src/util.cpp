// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "util.h"

static u16 NotePitchTable[] = {
    0x8000, 0x879C, 0x8FAC, 0x9837, 0xA145, 0xAADC, 0xB504,
    0xBFC8, 0xCB2F, 0xD744, 0xE411, 0xF1A1, 0x8000, 0x800E,
    0x801D, 0x802C, 0x803B, 0x804A, 0x8058, 0x8067, 0x8076,
    0x8085, 0x8094, 0x80A3, 0x80B1, 0x80C0, 0x80CF, 0x80DE,
    0x80ED, 0x80FC, 0x810B, 0x811A, 0x8129, 0x8138, 0x8146,
    0x8155, 0x8164, 0x8173, 0x8182, 0x8191, 0x81A0, 0x81AF,
    0x81BE, 0x81CD, 0x81DC, 0x81EB, 0x81FA, 0x8209, 0x8218,
    0x8227, 0x8236, 0x8245, 0x8254, 0x8263, 0x8272, 0x8282,
    0x8291, 0x82A0, 0x82AF, 0x82BE, 0x82CD, 0x82DC, 0x82EB,
    0x82FA, 0x830A, 0x8319, 0x8328, 0x8337, 0x8346, 0x8355,
    0x8364, 0x8374, 0x8383, 0x8392, 0x83A1, 0x83B0, 0x83C0,
    0x83CF, 0x83DE, 0x83ED, 0x83FD, 0x840C, 0x841B, 0x842A,
    0x843A, 0x8449, 0x8458, 0x8468, 0x8477, 0x8486, 0x8495,
    0x84A5, 0x84B4, 0x84C3, 0x84D3, 0x84E2, 0x84F1, 0x8501,
    0x8510, 0x8520, 0x852F, 0x853E, 0x854E, 0x855D, 0x856D,
    0x857C, 0x858B, 0x859B, 0x85AA, 0x85BA, 0x85C9, 0x85D9,
    0x85E8, 0x85F8, 0x8607, 0x8617, 0x8626, 0x8636, 0x8645,
    0x8655, 0x8664, 0x8674, 0x8683, 0x8693, 0x86A2, 0x86B2,
    0x86C1, 0x86D1, 0x86E0, 0x86F0, 0x8700, 0x870F, 0x871F,
    0x872E, 0x873E, 0x874E, 0x875D, 0x876D, 0x877D, 0x878C
};

static vol_pair PanTable[] = {
    { 0x3fff, 0x0000 },
    { 0x3ffe, 0x008e },
    { 0x3ffc, 0x011d },
    { 0x3ff9, 0x01ac },
    { 0x3ff5, 0x023b },
    { 0x3fef, 0x02ca },
    { 0x3fe8, 0x0359 },
    { 0x3fe0, 0x03e8 },
    { 0x3fd7, 0x0476 },
    { 0x3fcc, 0x0505 },
    { 0x3fc0, 0x0593 },
    { 0x3fb3, 0x0622 },
    { 0x3fa5, 0x06b0 },
    { 0x3f95, 0x073e },
    { 0x3f84, 0x07cc },
    { 0x3f72, 0x085a },
    { 0x3f5f, 0x08e8 },
    { 0x3f4b, 0x0975 },
    { 0x3f35, 0x0a02 },
    { 0x3f1e, 0x0a8f },
    { 0x3f06, 0x0b1c },
    { 0x3eec, 0x0ba9 },
    { 0x3ed1, 0x0c36 },
    { 0x3eb6, 0x0cc2 },
    { 0x3e98, 0x0d4e },
    { 0x3e7a, 0x0dd9 },
    { 0x3e5b, 0x0e65 },
    { 0x3e3a, 0x0ef0 },
    { 0x3e18, 0x0f7b },
    { 0x3df5, 0x1005 },
    { 0x3dd0, 0x1090 },
    { 0x3dab, 0x111a },
    { 0x3d84, 0x11a3 },
    { 0x3d5c, 0x122d },
    { 0x3d33, 0x12b5 },
    { 0x3d08, 0x133e },
    { 0x3cdd, 0x13c6 },
    { 0x3cb0, 0x144e },
    { 0x3c82, 0x14d5 },
    { 0x3c53, 0x155c },
    { 0x3c22, 0x15e3 },
    { 0x3bf1, 0x1669 },
    { 0x3bbe, 0x16ef },
    { 0x3b8b, 0x1774 },
    { 0x3b56, 0x17f9 },
    { 0x3b1f, 0x187d },
    { 0x3ae8, 0x1901 },
    { 0x3ab0, 0x1984 },
    { 0x3a76, 0x1a07 },
    { 0x3a3b, 0x1a89 },
    { 0x3a00, 0x1b0b },
    { 0x39c3, 0x1b8d },
    { 0x3984, 0x1c0d },
    { 0x3945, 0x1c8e },
    { 0x3905, 0x1d0d },
    { 0x38c3, 0x1d8c },
    { 0x3881, 0x1e0b },
    { 0x383d, 0x1e89 },
    { 0x37f8, 0x1f06 },
    { 0x37b3, 0x1f83 },
    { 0x376c, 0x1fff },
    { 0x3724, 0x207b },
    { 0x36da, 0x20f5 },
    { 0x3690, 0x2170 },
    { 0x3645, 0x21e9 },
    { 0x35f9, 0x2262 },
    { 0x35ab, 0x22da },
    { 0x355d, 0x2352 },
    { 0x350e, 0x23c9 },
    { 0x34bd, 0x243f },
    { 0x346c, 0x24b4 },
    { 0x3419, 0x2529 },
    { 0x33c6, 0x259d },
    { 0x3371, 0x2610 },
    { 0x331c, 0x2683 },
    { 0x32c5, 0x26f5 },
    { 0x326d, 0x2766 },
    { 0x3215, 0x27d6 },
    { 0x31bb, 0x2846 },
    { 0x3161, 0x28b4 },
    { 0x3106, 0x2922 },
    { 0x30a9, 0x298f },
    { 0x304c, 0x29fc },
    { 0x2fee, 0x2a67 },
    { 0x2f8e, 0x2ad2 },
    { 0x2f2e, 0x2b3c },
    { 0x2ecd, 0x2ba5 },
    { 0x2e6b, 0x2c0d },
    { 0x2e08, 0x2c74 },
    { 0x2da5, 0x2cda },
    { 0x2d40, 0x2d40 },
    { 0x2cda, 0x2da5 },
    { 0x2c74, 0x2e08 },
    { 0x2c0d, 0x2e6b },
    { 0x2ba5, 0x2ecd },
    { 0x2b3c, 0x2f2e },
    { 0x2ad2, 0x2f8e },
    { 0x2a67, 0x2fee },
    { 0x29fc, 0x304c },
    { 0x298f, 0x30a9 },
    { 0x2922, 0x3106 },
    { 0x28b4, 0x3161 },
    { 0x2846, 0x31bb },
    { 0x27d6, 0x3215 },
    { 0x2766, 0x326d },
    { 0x26f5, 0x32c5 },
    { 0x2683, 0x331c },
    { 0x2610, 0x3371 },
    { 0x259d, 0x33c6 },
    { 0x2529, 0x3419 },
    { 0x24b4, 0x346c },
    { 0x243f, 0x34bd },
    { 0x23c9, 0x350e },
    { 0x2352, 0x355d },
    { 0x22da, 0x35ab },
    { 0x2262, 0x35f9 },
    { 0x21e9, 0x3645 },
    { 0x2170, 0x3690 },
    { 0x20f5, 0x36da },
    { 0x207b, 0x3724 },
    { 0x1fff, 0x376c },
    { 0x1f83, 0x37b3 },
    { 0x1f06, 0x37f8 },
    { 0x1e89, 0x383d },
    { 0x1e0b, 0x3881 },
    { 0x1d8c, 0x38c3 },
    { 0x1d0d, 0x3905 },
    { 0x1c8e, 0x3945 },
    { 0x1c0d, 0x3984 },
    { 0x1b8d, 0x39c3 },
    { 0x1b0b, 0x3a00 },
    { 0x1a89, 0x3a3b },
    { 0x1a07, 0x3a76 },
    { 0x1984, 0x3ab0 },
    { 0x1901, 0x3ae8 },
    { 0x187d, 0x3b1f },
    { 0x17f9, 0x3b56 },
    { 0x1774, 0x3b8b },
    { 0x16ef, 0x3bbe },
    { 0x1669, 0x3bf1 },
    { 0x15e3, 0x3c22 },
    { 0x155c, 0x3c53 },
    { 0x14d5, 0x3c82 },
    { 0x144e, 0x3cb0 },
    { 0x13c6, 0x3cdd },
    { 0x133e, 0x3d08 },
    { 0x12b5, 0x3d33 },
    { 0x122d, 0x3d5c },
    { 0x11a3, 0x3d84 },
    { 0x111a, 0x3dab },
    { 0x1090, 0x3dd0 },
    { 0x1005, 0x3df5 },
    { 0x0f7b, 0x3e18 },
    { 0x0ef0, 0x3e3a },
    { 0x0e65, 0x3e5b },
    { 0x0dd9, 0x3e7a },
    { 0x0d4e, 0x3e98 },
    { 0x0cc2, 0x3eb6 },
    { 0x0c36, 0x3ed1 },
    { 0x0ba9, 0x3eec },
    { 0x0b1c, 0x3f06 },
    { 0x0a8f, 0x3f1e },
    { 0x0a02, 0x3f35 },
    { 0x0975, 0x3f4b },
    { 0x08e8, 0x3f5f },
    { 0x085a, 0x3f72 },
    { 0x07cc, 0x3f84 },
    { 0x073e, 0x3f95 },
    { 0x06b0, 0x3fa5 },
    { 0x0622, 0x3fb3 },
    { 0x0593, 0x3fc0 },
    { 0x0505, 0x3fcc },
    { 0x0476, 0x3fd7 },
    { 0x03e8, 0x3fe0 },
    { 0x0359, 0x3fe8 },
    { 0x02ca, 0x3fef },
    { 0x023b, 0x3ff5 },
    { 0x01ac, 0x3ff9 },
    { 0x011d, 0x3ffc },
    { 0x008e, 0x3ffe },
};

vol_pair make_volume(int sound_vol, int velocity_volume, int pan, int prog_vol, int prog_pan, int tone_vol, int tone_pan)
{
    s32 vol = (((((velocity_volume * 0x7f * tone_vol) / 0x7f) * prog_vol) / 0x7f) * sound_vol) / 0x7f;
    // s32 vol = (velocity_volume * tone_vol * prog_vol * sound_vol) >> 7;
    // fmt::print("cvol {:x} nvol {:x} pvol {:x} tvol {:x} -> {:x}\n", sound_vol, velocity_volume, prog_vol, tone_vol, vol);

    // fmt::print("pan1 {} pan2 {} pan3 {}\n", pan, prog_pan, tone_pan);
    if (vol == 0) {
        return { 0, 0 };
    }

    int total_pan = pan + tone_pan + prog_pan;
    // while (total_pan >= 360) {
    //     total_pan -= 360;
    // }
    // while (total_pan < 0) {
    //     total_pan += 360;
    // }
    // if (total_pan >= 270) {
    //     total_pan -= 270;
    // } else {
    //     total_pan += 90;
    // }
    while (total_pan < 0) {
        total_pan += 360;
    }
    if (total_pan >= 270) {
        total_pan -= 270;
    } else {
        total_pan += 90;
    }

    // fmt::print("total pan {}\n", total_pan);

    if (total_pan >= 180) {
        s16 lvol = (PanTable[total_pan - 180].left * vol) >> 15;
        s16 rvol = (PanTable[total_pan - 180].right * vol) >> 15;
        if (lvol < 0 && rvol < 0) {
            lvol = -lvol;
            rvol = -rvol;
        }

        // fmt::print("lvol {:x} rvol {:x}\n", lvol, rvol);
        return { lvol, rvol };
    } else {
        s16 rvol = (PanTable[total_pan].left * vol) >> 15;
        s16 lvol = (PanTable[total_pan].right * vol) >> 15;
        if (lvol < 0 && rvol < 0) {
            lvol = -lvol;
            rvol = -rvol;
        }

        // fmt::print("lvol {:x} rvol {:x}\n", lvol, rvol);
        return { lvol, rvol };
    }

    // TODO rest of this function

    return { (s16)vol, (s16)vol };
}

u16 sceSdNote2Pitch(u16 center_note, u16 center_fine, u16 note, short fine)
{
    s32 _fine;
    s32 _fine2;
    s32 _note;
    s32 offset1, offset2;
    s32 val;
    s32 val2;
    s32 val3;
    s32 ret;

    _fine = fine + (u16)center_fine;
    _fine2 = _fine;

    if (_fine < 0)
        _fine2 = _fine + 127;

    _fine2 = _fine2 / 128;
    _note = note + _fine2 - center_note;
    val3 = _note / 6;

    if (_note < 0)
        val3--;

    offset2 = _fine - _fine2 * 128;

    if (_note < 0)
        val2 = -1;
    else
        val2 = 0;
    if (val3 < 0)
        val3--;

    val2 = (val3 / 2) - val2;
    val = val2 - 2;
    offset1 = _note - (val2 * 12);

    if ((offset1 < 0) || ((offset1 == 0) && (offset2 < 0))) {
        offset1 = offset1 + 12;
        val = val2 - 3;
    }

    if (offset2 < 0) {
        offset1 = (offset1 - 1) + _fine2;
        offset2 += (_fine2 + 1) * 128;
    }

    ret = (NotePitchTable[offset1] * NotePitchTable[offset2 + 12]) / 0x10000;

    if (val < 0)
        ret = (ret + (1 << (-val - 1))) >> -val;

    return (u16)ret;
}

u16 PS1Note2Pitch(s8 center_note, s8 center_fine, short note, short fine)
{
    bool thing = false;
    if (center_note >= 0) {
        thing = true;
    } else {
        thing = false;
        center_note = -center_note;
    }

    auto pitch = sceSdNote2Pitch(center_note, center_fine, note, fine);
    if (thing) {
        pitch = 44100 * pitch / 48000;
    }

    // fmt::print("center_note {}, center_fine {}, note {}, fine {} -> pitch {:04x}\n", center_note, center_fine, note, fine, pitch);
    return pitch;
}
