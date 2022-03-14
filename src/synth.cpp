#include "synth.h"
#include "src/player.h"

s16_output synth::tick()
{
    s16_output out {};
    for (auto& v : m_voices) {
        out += v.get()->run();
    }

    // clean up disused ones?

    return out;
}

void synth::key_on(Tone& bank, u8 channel, u8 note, u8 velocity)
{
}

void synth::key_off(Tone& bank, u8 channel, u8 note, u8 velocity)
{
}
