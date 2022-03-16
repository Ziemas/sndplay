// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "midi_handler.h"
#include "sound_handler.h"
#include "types.h"
#include <array>

class midi_handler;
class ame_handler : public sound_handler {
public:
    bool tick() override;

    u8* run_ame(midi_handler&, u8* stream);

private:
    struct ame_error : public std::exception {
        ame_error(std::string text)
            : msg(std::move(text))
        {
        }
        ame_error()
            : msg("Unknown AME error")
        {
        }
        std::string msg;
        const char* what() const noexcept override
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

    std::forward_list<midi_handler> m_midis;

    u8 m_excite { 0 };
    std::array<GroupDescription, 16> m_groups {};
    std::array<u8, 16> m_register;
    std::array<u8*, 16> m_macro;
};
