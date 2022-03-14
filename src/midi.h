// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "SDL_audio.h"
#include "musplay.h"
#include "types.h"
#include "voice.h"
#include <exception>
#include <utility>

class midi_player {
public:
    midi_player(MIDIBlockHeader* block, u8* sample_data)
        : m_header(block)
        , m_sample_data(sample_data)
    {
        m_seq_data_start = (u8*)((uintptr_t)block + (uintptr_t)block->DataStart);
        m_seq_ptr = m_seq_data_start;
        m_tempo = block->Tempo;
        m_ppq = block->PPQ;
    };

    void start();

private:
    static constexpr int tickrate = 48000;
    // static constexpr int tickrate = 240;
    static constexpr int mics_per_tick = (100000000 / tickrate) / 100;
    struct midi_error : public std::exception {
        midi_error(std::string text)
            : msg(std::move(text))
        {
        }
        midi_error()
            : msg("Unknown MIDI error")
        {
        }
        std::string msg;
        const char* what() const throw() override
        {
            return msg.c_str();
        }
    };

    struct GroupDescription {
        /*   0 */ s8 num_channels;
        /*   1 */ s8 basis;
        /*   2 */ s8 pad[2];
        /*   4 */ s8 channel[16];
        /*  14 */ s8 excite_min[16];
        /*  24 */ s8 excite_max[16];
    };

    enum class ame_op : u8 {
        // Compares excite to next byte and puts result in the flag
        cmp_excite_less = 0x0,
        cmp_excite_not_equal = 0x1,
        cmp_excite_greater = 0x2,

        // if the comparison flag is set, call func
        cond_stop_stream = 0x3,

        unk = 0x4, // flips flag to two if one
        unk2 = 0x5, // flips flag to 0 if 2

        comp_midireg_greater = 0x6,
        comp_midireg_less = 0x7,

        store_macro = 0xb,

        cond_run_macro = 0xc,
        read_group_data = 0xf,
        cond_stop_and_start = 0x11,
        cond_start_segment = 0x12,
        cond_set_reg = 0x13,

    };

    MIDIBlockHeader* m_header { nullptr };
    SDL_AudioDeviceID m_dev;

    u8* m_sample_data { nullptr };

    std::array<voice, 16> m_voices;

    u8* m_seq_data_start { nullptr };
    u8* m_seq_ptr { nullptr };
    u8 m_status { 0 };
    u32 m_tickrate { 240 };
    u32 m_tempo { 500000 };
    u32 m_ppq { 480 };
    u32 m_time { 0 };
    u32 m_tickerror { 0 };
    u32 m_tickdelta { 0 };
    u32 m_ppt { 0 };
    u64 m_tick_countdown { 0 };
    bool m_get_delta { true };
    bool m_track_complete { false };

    u8* m_macro[16];
    std::array<GroupDescription, 16> m_groups;
    std::array<u8, 16> m_programs;

    u8 m_register[16];
    u8 m_excite { 0 };

    static void sdl_callback(void* userdata, u8* stream, int len);

    void play(u8* output, int len);

    void step();
    void new_delta(bool reset);

    void run_ame(u8* macro);
    void note_on();
    void note_off();
    void channel_pressure();
    void program_change();
    void meta_event();
    void system_event();
    void channel_pitch();

    static std::pair<size_t, u32> read_vlq(u8* value);
};
