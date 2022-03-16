// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "midi_handler.h"
#include "sound_handler.h"
#include "synth.h"
#include "types.h"
#include <array>
#include <forward_list>

struct MultiMIDIBlockHeader {
    /*   0 */ u32 DataID;
    /*   4 */ s16 Version;
    /*   6 */ s8 Flags;
    /*   7 */ s8 NumMIDIBlocks;
    /*   8 */ u32 ID;
    /*   c */ /*void**/ u32 NextMIDIBlock;
    /*  10 */ /*s8**/ u32 BlockPtr[1];
};

class midi_handler;
class ame_handler : public sound_handler {
public:
    ame_handler(MultiMIDIBlockHeader* block, synth& synth, s32 vol, s32 pan, s8 repeats, locator& loc);
    bool tick() override;

    std::pair<bool, u8*> run_ame(midi_handler&, u8* stream);

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

#pragma pack(push, 1)
    struct GroupDescription {
        /*   0 */ s8 num_channels;
        /*   1 */ s8 basis;
        /*   2 */ s8 pad[2];
        /*   4 */ s8 channel[16];
        /*  14 */ s8 excite_min[16];
        /*  24 */ s8 excite_max[16];
    };
#pragma pack(pop)

    enum class ame_op : u8 {
        // Compares excite to next byte and puts result in the flag
        cmp_excite_less = 0x0,
        cmp_excite_not_equal = 0x1,
        cmp_excite_greater = 0x2,

        // if the comparison flag is set, call func
        cond_stop_stream = 0x3,

        unk = 0x4, // flips flag to two if one
        unk2 = 0x5, // flips flag to 0 if 2

        cmp_midireg_greater = 0x6,
        cmp_midireg_less = 0x7,

        store_macro = 0xb,

        cond_run_macro = 0xc,
        read_group_data = 0xf,
        thing = 0x10,
        cond_stop_and_start = 0x11,
        cond_start_segment = 0x12,
        cond_set_reg = 0x13,
        cond_inc_reg = 0x14,

        cmp_reg_not_equal = 0x16,
    };

    void start_segment(u32 id);
    void stop_segment(u32 id);

    MultiMIDIBlockHeader* m_header { nullptr };
    locator& m_locator;
    synth& m_synth;
    s32 m_vol { 0 };
    s32 m_pan { 0 };
    s8 m_repeats { 0 };

    std::forward_list<std::unique_ptr<midi_handler>> m_midis;

    u8 m_excite { 0 };
    std::array<GroupDescription, 16> m_groups {};
    std::array<u8, 16> m_register {};
    std::array<u8*, 16> m_macro {};
};
