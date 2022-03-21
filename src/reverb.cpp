// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "Reverb.h"
#include "SpuCore.h"

namespace snd
{
	static constexpr u32 NUM_TAPS = 39;
	static constexpr std::array<s32, NUM_TAPS> FilterCoefficients = {
		-1,
		0,
		2,
		0,
		-10,
		0,
		35,
		0,
		-103,
		0,
		266,
		0,
		-616,
		0,
		1332,
		0,
		-2960,
		0,
		10246,
		16384,
		10246,
		0,
		-2960,
		0,
		1332,
		0,
		-616,
		0,
		266,
		0,
		-103,
		0,
		35,
		0,
		-10,
		0,
		2,
		0,
		-1,
	};

	s16 Reverb::DownSample(AudioSample in)
	{
		m_ReverbIn.Push(in);

		s32 down{0};
		for (u32 i = 0; i < NUM_TAPS; i++)
		{
			auto s = m_ReverbIn.Get(i);
			if (m_Phase)
				down += s.right * FilterCoefficients[i];
			else
				down += s.left * FilterCoefficients[i];
		}

		down >>= 15;
		return static_cast<s16>(std::clamp<s32>(down, INT16_MIN, INT16_MAX));
	}

	AudioSample Reverb::UpSample(s16 in)
	{
		AudioSample up(0, 0);

		if (m_Phase)
			up.right = in;
		else
			up.left = in;

		m_ReverbOut.Push(up);

		s32 left{0}, right{0};
		for (u32 i = 0; i < NUM_TAPS; i++)
		{
			left += m_ReverbOut.Get(i).left * FilterCoefficients[i];
			right += m_ReverbOut.Get(i).right * FilterCoefficients[i];
		}

		left >>= 14;
		right >>= 14;

		AudioSample out(static_cast<s16>(std::clamp<s32>(left, INT16_MIN, INT16_MAX)),
			static_cast<s16>(std::clamp<s32>(right, INT16_MIN, INT16_MAX)));

		return out;
	}

	static s32 IIASM(const s16 vIIR, const s16 sample)
	{
		if (vIIR == INT16_MIN)
		{
			if (sample == INT16_MIN)
				return 0;
			else
				return sample * -0x10000;
		}

		return sample * (INT16_MAX - vIIR);
	}

	static s16 ReverbSat(s32 sample)
	{
		return static_cast<s16>(std::clamp<s32>(sample, INT16_MIN, INT16_MAX));
	}

	static s16 ReverbNeg(s16 sample)
	{
		if (sample == INT16_MIN)
			return 0x7FFF;

		return static_cast<s16>(-sample);
	}

	u32 Reverb::Offset(s32 offset) const
	{
		uint32_t address = m_pos + offset;
		uint32_t size = m_EEA.full - m_ESA.full;

		if (size == 0)
			return 0;

		address = m_ESA.full + (address % size);

		return address;
	}

	s16 Reverb::RD_RVB(s32 address, s32 offset)
	{
		m_SPU.TestIrq(Offset(address + offset));
		return static_cast<s16>(m_SPU.Ram(Offset(address + offset)));
	}

	void Reverb::WR_RVB(s32 address, s16 sample)
	{
		if (m_Enable)
		{
			m_SPU.TestIrq(Offset(address));
			m_SPU.WriteMem(Offset(address), static_cast<u16>(sample));
		}
	}

	AudioSample Reverb::Run(AudioSample input)
	{
		// down-sample input
		auto in = DownSample(input);

		const s16 SAME_SIDE_IN = ReverbSat((((RD_RVB(static_cast<s32>(dSAME[m_Phase ^ 0].full)) * vWALL) >> 14) + ((in * vIN[m_Phase]) >> 14)) >> 1);
		const s16 DIFF_SIDE_IN = ReverbSat((((RD_RVB(static_cast<s32>(dDIFF[m_Phase ^ 1].full)) * vWALL) >> 14) + ((in * vIN[m_Phase]) >> 14)) >> 1);
		const s16 SAME_SIDE = ReverbSat((((SAME_SIDE_IN * vIIR) >> 14) + (IIASM(vIIR, RD_RVB(static_cast<s32>(mSAME[m_Phase].full), -1)) >> 14)) >> 1);
		const s16 DIFF_SIDE = ReverbSat((((DIFF_SIDE_IN * vIIR) >> 14) + (IIASM(vIIR, RD_RVB(static_cast<s32>(mDIFF[m_Phase].full), -1)) >> 14)) >> 1);

		WR_RVB(static_cast<s32>(mSAME[m_Phase].full), SAME_SIDE);
		WR_RVB(static_cast<s32>(mDIFF[m_Phase].full), DIFF_SIDE);

		const s32 COMB = ((RD_RVB(static_cast<s32>(mCOMB1[m_Phase].full)) * vCOMB1) >> 14) +
						 ((RD_RVB(static_cast<s32>(mCOMB2[m_Phase].full)) * vCOMB2) >> 14) +
						 ((RD_RVB(static_cast<s32>(mCOMB3[m_Phase].full)) * vCOMB3) >> 14) +
						 ((RD_RVB(static_cast<s32>(mCOMB4[m_Phase].full)) * vCOMB4) >> 14);

		const s16 APF1 = RD_RVB(static_cast<s32>(mAPF1[m_Phase].full - dAPF[0].full));
		const s16 APF2 = RD_RVB(static_cast<s32>(mAPF2[m_Phase].full - dAPF[1].full));
		const s16 MDA = ReverbSat((COMB + ((APF1 * ReverbNeg(vAPF1)) >> 14)) >> 1);
		const s16 MDB = ReverbSat(APF1 + ((((MDA * vAPF1) >> 14) + ((APF2 * ReverbNeg(vAPF2)) >> 14)) >> 1));
		const s16 IVB = ReverbSat(APF2 + ((MDB * vAPF2) >> 15));

		WR_RVB(static_cast<s32>(mAPF1[m_Phase].full), MDA);
		WR_RVB(static_cast<s32>(mAPF2[m_Phase].full), MDB);

		// up-sample output
		auto output = UpSample(IVB);

		m_Phase ^= 1;
		if (m_Phase)
			m_pos++;
		if (m_pos >= m_EEA.full - m_ESA.full + 1)
			m_pos = 0;

		return output;
	}
} // namespace SPU
