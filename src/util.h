// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once

#include "common/Bitfield.h"
#include "common/Pcsx2Types.h"
#include "Envelope.h"
#include <algorithm>

namespace SPU
{
	__fi static void SET_HIGH(u32& number, u32 upper) { number = (number & 0x0000FFFF) | upper << 16; }
	__fi static void SET_LOW(u32& number, u32 low) { number = (number & 0xFFFF0000) | low; }
	__fi static u32 GET_HIGH(u32 number) { return (number & 0xFFFF0000) >> 16; }
	__fi static u32 GET_LOW(u32 number) { return number & 0x0000FFFF; }
	__fi static u32 GET_BIT(u32 idx, u32 value) { return (value >> idx) & 1; }
	__fi static void SET_BIT(u32& number, u32 idx) { number |= (1 << idx); }

	__fi static s16 ApplyVolume(s16 sample, s32 volume)
	{
		return (sample * volume) >> 15;
	}

    struct PlainVolReg
    {
        s16 left;
        s16 right;
    };

	struct AudioSample
	{
		AudioSample() = default;
		AudioSample(s16 left, s16 right)
			: left(left)
			, right(right)
		{
		}

		s16 left{0};
		s16 right{0};

		void Mix(AudioSample src, bool lgate, bool rgate)
		{
			if (lgate)
				left = static_cast<s16>(std::clamp<s32>(left + src.left, INT16_MIN, INT16_MAX));
			if (rgate)
				right = static_cast<s16>(std::clamp<s32>(right + src.right, INT16_MIN, INT16_MAX));
		}

		void Volume(PlainVolReg vol)
		{
			left = ApplyVolume(left, vol.left);
			right = ApplyVolume(right, vol.right);
		}

		void Volume(VolumePair vol)
		{
			left = ApplyVolume(left, vol.left.GetCurrent());
			right = ApplyVolume(right, vol.right.GetCurrent());
		}
	};

	union Reg32
	{
		u32 full;

		BitField<u32, u16, 16, 16> hi;
		BitField<u32, u16, 0, 16> lo;
	};
} // namespace SPU
