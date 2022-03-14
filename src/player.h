#include "types.h"
#include <filesystem>
#include <forward_list>
#include <memory>
#include <unordered_map>
#include <vector>

struct Tone {
    /*   0 */ s8 Priority;
    /*   1 */ s8 Vol;
    /*   2 */ s8 CenterNote;
    /*   3 */ s8 CenterFine;
    /*   4 */ s16 Pan;
    /*   6 */ s8 MapLow;
    /*   7 */ s8 MapHigh;
    /*   8 */ s8 PBLow;
    /*   9 */ s8 PBHigh;
    /*   a */ s16 ADSR1;
    /*   c */ s16 ADSR2;
    /*   e */ s16 Flags;
    /*  10 */ /*void**/ u32 VAGInSR;
    /*  14 */ u32 reserved1;
};

struct ProgData {
    /*   0 */ s8 NumTones;
    /*   1 */ s8 Vol;
    /*   2 */ s16 Pan;
    /*   4 */ /*Tone**/ u32 FirstTone;
};

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

struct MultiMIDIBlockHeader {
    /*   0 */ u32 DataID;
    /*   4 */ s16 Version;
    /*   6 */ s8 Flags;
    /*   7 */ s8 NumMIDIBlocks;
    /*   8 */ u32 ID;
    /*   c */ /*void**/ u32 NextMIDIBlock;
    /*  10 */ /*s8**/ u32 BlockPtr[1];
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

struct FileAttributes {
    /*   0 */ u32 type;
    /*   4 */ u32 num_chunks;
    /*   8 */ LocAndSize where[0];
};

struct Prog {
    ProgData d;
    std::vector<Tone> tones;
};

struct SoundBank {
    SoundBankData d;
    std::vector<Prog> programs;
    std::vector<MIDISound> sounds;
    std::unique_ptr<u8[]> sampleBuf;
};

class snd_player {
public:
    u32 load_bank(std::filesystem::path path);
    void load_midi(std::unique_ptr<u8[]> midi);
    void play_sound(u32 bank, u32 sound);

private:
    std::unordered_map<u32, SoundBank> m_soundbanks;
    std::vector<SoundBank> m_soundblocks;
    std::unordered_map<u32, MIDIBlockHeader> m_midi;
};
