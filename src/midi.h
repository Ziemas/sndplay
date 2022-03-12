#pragma once
#include "synth.h"
#include "types.h"
#include <exception>
#include <utility>

class midi_player {
public:
    midi_player(u8* sequence_data, u8* sample_data)
        : m_synth(sample_data)
        , m_seq_data_start(sequence_data)
        , m_seq_ptr(sequence_data) {};
    void play();

private:
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

    };

    synth m_synth;

    u8* m_seq_data_start { nullptr };
    u8* m_seq_ptr { nullptr };
    u8 m_status { 0 };
    u32 m_time { 0 };
    u32 m_tick { 0 };
    u32 m_tickrate { 240 };
    bool m_track_complete { false };

    u8* m_macro[16];

    u8 m_register[16];
    u8 m_excite { 0 };

    void step();

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
