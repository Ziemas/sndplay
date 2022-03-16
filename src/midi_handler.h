// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "ame_handler.h"
#include "player.h"
#include "sound_handler.h"
#include "synth.h"
#include "types.h"
#include "voice.h"
#include <exception>
#include <optional>
#include <string>
#include <utility>

struct MIDIBlockHeader {
    /*   0 */ u32 DataID;
    /*   4 */ s16 Version;
    /*   6 */ s8 Flags;
    /*   7 */ s8 pad1;
    /*   8 */ u32 ID;
    /*   c */ /*void**/ u32 NextMIDIBlock;
    /*  10 */ u32 BankID;
    /*  14 */ /*SoundBank**/ u32 BankPtr;
    /*  18 */ /*s8**/ u32 DataStart;
    /*  1c */ /*s8**/ u32 MultiMIDIParent;
    /*  20 */ u32 Tempo;
    /*  24 */ u32 PPQ;
};

class ame_handler;

class midi_handler : public sound_handler {
public:
    midi_handler(MIDIBlockHeader* block, synth& synth, s32 vol, s32 pan, s8 repeats, locator& loc)
        : m_locator(loc)
        , m_vol(vol)
        , m_pan(pan)
        , m_repeats(repeats)
        , m_header(block)
        , m_synth(synth)
    {
        m_seq_data_start = (u8*)((uintptr_t)block + (uintptr_t)block->DataStart);
        m_seq_ptr = m_seq_data_start;
        m_tempo = block->Tempo;
        m_ppq = block->PPQ;
    };

    midi_handler(MIDIBlockHeader* block, synth& synth, s32 vol, s32 pan, s8 repeats, locator& loc, ame_handler* parent)
        : m_parent(parent)
        , m_locator(loc)
        , m_vol(vol)
        , m_pan(pan)
        , m_repeats(repeats)
        , m_header(block)
        , m_synth(synth)
    {
        // fmt::print("spawning midi handler at {:x}\n", (u64)this);
        m_seq_data_start = (u8*)((uintptr_t)block + (uintptr_t)block->DataStart);
        m_seq_ptr = m_seq_data_start;
        m_tempo = block->Tempo;
        m_ppq = block->PPQ;

        fmt::print("added new midi handler {:x}\n", (u64)this);
        fmt::print("tempo:{:x} ppq:{} repeats:{} \n", m_tempo, m_ppq, repeats);

    };

    void start();
    bool tick() override;
    void mute_channel(u8 channel);
    void unmute_channel(u8 channel);

    bool complete() { return m_track_complete; };

private:
    static constexpr int tickrate = 240;
    static constexpr int mics_per_tick = 1000000 / tickrate;
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
        const char* what() const noexcept override
        {
            return msg.c_str();
        }
    };

    std::optional<ame_handler*> m_parent;

    locator& m_locator;
    s32 m_vol { 0 };
    s32 m_pan { 0 };
    s8 m_repeats { 0 };

    MIDIBlockHeader* m_header { nullptr };

    std::array<bool, 16> m_mute_state {};
    u8* m_sample_data { nullptr };

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
    u32 m_muted_channels { 0 };

    std::array<u8, 16> m_programs {};

    synth& m_synth;

    void step();
    void new_delta(bool reset);

    void note_on();
    void note_off();
    void channel_pressure();
    void program_change();
    void meta_event();
    void system_event();
    void channel_pitch();

    static std::pair<size_t, u32> read_vlq(u8* value);
};
