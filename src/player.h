// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "midi_handler.h"
#include "ame_handler.h"
#include "sound_handler.h"
#include "synth.h"
#include "types.h"
#include <SDL.h>
#include <filesystem>
#include <forward_list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

struct MIDISound {
    /*   0 */ s32 Type;
    /*   4 */ /*SoundBank**/ u32 Bank;
    /*   8 */ /*void**/ u32 OrigBank;
    /*   c */ u32 MIDIID;
    /*  10 */ s16 Vol;
    /*  12 */ s8 Repeats;
    /*  13 */ s8 VolGroup;
    /*  14 */ s16 Pan;
    /*  16 */ s8 Index;
    /*  17 */ s8 Flags;
    /*  18 */ /*void**/ u32 MIDIBlock;
};

struct SoundBankData {
    /*   0 */ u32 DataID;
    /*   4 */ u32 Version;
    /*   8 */ u32 Flags;
    /*   c */ u32 BankID;
    /*  10 */ s8 BankNum;
    /*  11 */ s8 pad1;
    /*  12 */ s16 pad2;
    /*  14 */ s16 NumSounds;
    /*  16 */ s16 NumProgs;
    /*  18 */ s16 NumTones;
    /*  1a */ s16 NumVAGs;
    /*  1c */ /*Sound**/ u32 FirstSound;
    /*  20 */ /*Prog**/ u32 FirstProg;
    /*  24 */ /*Tone**/ u32 FirstTone;
    /*  28 */ /*void**/ u32 VagsInSR;
    /*  2c */ u32 VagDataSize;
    /*  30 */ /*SoundBank**/ u32 NextBank;
};
enum class file_chunk : u32 {
    bank,
    samples,
    midi
};

struct LocAndSize {
    /*   0 */ u32 offset;
    /*   4 */ u32 size;
};

template <size_t chunks>
struct FileAttributes {
    /*   0 */ u32 type;
    /*   4 */ u32 num_chunks;
    /*   8 */ LocAndSize where[chunks];
};

struct Prog;
struct SoundBank {
    SoundBankData d;
    std::vector<Prog> programs;
    std::vector<MIDISound> sounds;
    std::unique_ptr<u8[]> sampleBuf;
};

struct MIDIBlockHeader;

class snd_player : public locator {
public:
    snd_player();
    ~snd_player();
    //snd_player(const snd_player&) = delete;
    //snd_player operator=(const snd_player&) = delete;

    //snd_player(snd_player&& other) noexcept = default;
    //snd_player& operator=(snd_player&& other) noexcept = default;

    u32 load_bank(std::filesystem::path path);
    void load_midi(std::fstream& in);
    void play_sound(u32 bank, u32 sound);

    // TODO this shouldn't be public, figure something out
    void tick(s16_output* stream, int samples);


    SoundBank& get_bank(u32 id) override;
private:
    std::recursive_mutex m_ticklock; // TODO does not need to recursive with some light restructuring
    std::forward_list<std::unique_ptr<sound_handler>> m_handlers;
    std::unordered_map<u32, SoundBank> m_soundbanks;
    std::vector<SoundBank> m_soundblocks;
    std::vector<std::unique_ptr<u8[]>> m_midi_chunks;
    std::unordered_map<u32, MIDIBlockHeader*> m_midi;

    void play_midi(MIDISound& sound, s32 vol, s32 pan);

    synth m_synth;

    SDL_AudioDeviceID m_dev {};
    static void sdl_callback(void* userdata, u8* stream, int len);
};
