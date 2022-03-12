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
    struct unknown_event : public std::exception {
        const char* what() const throw() override
        {
            return "Unknown MIDI event";
        }
    };

    synth m_synth;

    u8* m_seq_data_start { nullptr };
    u8* m_seq_ptr { nullptr };
    u8 m_status { 0 };
    u32 m_time { 0 };
    u32 m_tick { 0 };
    u32 m_tickrate { 240 };
    bool m_track_complete { false };

    void step();

    void note_on();
    void note_off();
    void channel_pressure();
    void program_change();
    void meta_event();
    void system_event();
    void channel_pitch();

    static std::pair<size_t, u32> read_vlq(u8* value);
};
