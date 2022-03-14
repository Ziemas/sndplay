#include "synth.h"

s16_output synth::tick()
{
    s16_output out {};
    for (auto& v : m_voices) {
        out += v.get()->run();
    }

    // clean up disused ones?

    return out;
}

void synth::key_on(u32 bank, u8 channel, u8 note, u8 velocity)
{
}

void synth::key_off(u32 bank, u8 channel, u8 note, u8 velocity)
{
}
