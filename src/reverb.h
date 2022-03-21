// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once

#include "fifo.h"
#include "util.h"

namespace snd
{
	class SPUCore;

	class Reverb
	{
	public:
		explicit Reverb(SPUCore& core)
			: m_SPU(core){};

		AudioSample Run(AudioSample input);

		bool m_Enable{false};

		Reg32 m_ESA{0};
		Reg32 m_EEA{0};
		u32 m_pos{0};

		Reg32 dAPF[2]{{0}, {0}};
		Reg32 mSAME[2]{{0}, {0}};
		Reg32 mCOMB1[2]{{0}, {0}};
		Reg32 mCOMB2[2]{{0}, {0}};
		Reg32 dSAME[2]{{0}, {0}};
		Reg32 mDIFF[2]{{0}, {0}};
		Reg32 mCOMB3[2]{{0}, {0}};
		Reg32 mCOMB4[2]{{0}, {0}};
		Reg32 dDIFF[2]{{0}, {0}};
		Reg32 mAPF1[2]{{0}, {0}};
		Reg32 mAPF2[2]{{0}, {0}};

		s16 vIIR{0};
		s16 vCOMB1{0};
		s16 vCOMB2{0};
		s16 vCOMB3{0};
		s16 vCOMB4{0};
		s16 vWALL{0};
		s16 vAPF1{0};
		s16 vAPF2{0};
		s16 vIN[2]{0};

		void Reset()
		{
			for (auto& r : dAPF)
				r.full = 0;
			for (auto& r : mSAME)
				r.full = 0;
			for (auto& r : mCOMB1)
				r.full = 0;
			for (auto& r : mCOMB2)
				r.full = 0;
			for (auto& r : dSAME)
				r.full = 0;
			for (auto& r : mDIFF)
				r.full = 0;
			for (auto& r : mCOMB3)
				r.full = 0;
			for (auto& r : mCOMB4)
				r.full = 0;
			for (auto& r : dDIFF)
				r.full = 0;
			for (auto& r : mAPF1)
				r.full = 0;
			for (auto& r : mAPF2)
				r.full = 0;
			for (auto& r : vIN)
				r = 0;
			vIIR = 0;
			vCOMB1 = 0;
			vCOMB2 = 0;
			vCOMB3 = 0;
			vCOMB4 = 0;
			vWALL = 0;
			vAPF1 = 0;
			vAPF2 = 0;
			m_Phase = 0;
			m_pos = 0;
		}

	private:
		static constexpr u32 NUM_TAPS = 39;

		SPUCore& m_SPU;

		template <size_t len>
		struct SampleBuffer
		{
			u32 m_Pos{0};
			std::array<AudioSample, len> m_Buffer{};

			void Push(AudioSample sample)
			{
				m_Pos = (m_Pos + 1) % len;
				m_Buffer[m_Pos] = sample;
			}

			[[nodiscard]] const AudioSample& Get(u32 index) const
			{
				return m_Buffer[(m_Pos + index + 1) % len];
			}
		};
		SampleBuffer<NUM_TAPS> m_ReverbIn{};
		SampleBuffer<NUM_TAPS> m_ReverbOut{};

		s16 DownSample(AudioSample in);
		AudioSample UpSample(s16 in);

		s16 RD_RVB(s32 address, s32 offset = 0);
		void WR_RVB(s32 address, s16 sample);
		[[nodiscard]] u32 Offset(s32 offset) const;

		u32 m_Phase{0};
	};
} // namespace SPU
