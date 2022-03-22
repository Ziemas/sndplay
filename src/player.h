// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "ame_handler.h"
#include "cubeb/cubeb.h"
#include "midi_handler.h"
#include "sound_handler.h"
#include "loader.h"
#include "synth.h"
#include "types.h"
#include <filesystem>
#include <forward_list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace snd {

class player {
public:
    player();
    ~player();
    player(const player&) = delete;
    player operator=(const player&) = delete;

    // player(player&& other) noexcept = default;
    // player& operator=(player&& other) noexcept = default;

    u32 load_bank(std::filesystem::path& path, size_t offset);

    void play_sound(u32 bank, u32 sound);
    void set_midi_reg(u8 reg, u8 value);

private:
    std::recursive_mutex m_ticklock; // TODO does not need to recursive with some light restructuring
    std::forward_list<std::unique_ptr<sound_handler>> m_handlers;

    void play_midi(MIDISound& sound, s32 vol, s32 pan);
    void play_ame(MIDISound& sound, s32 vol, s32 pan);
    void tick(s16_output* stream, int samples);

    loader m_loader;
    synth m_synth;

    cubeb* m_ctx { nullptr };
    cubeb_stream* m_stream { nullptr };

    static long sound_callback(cubeb_stream* stream, void* user, const void* input, void* output_buffer, long len);
    static void state_callback(cubeb_stream* stream, void* user, cubeb_state state);
};
}
