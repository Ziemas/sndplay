#include "synth.h"
#include "player.h"
#include "util.h"

void synth::load_samples(u32 bank, std::unique_ptr<u8[]> samples)
{
    // m_bank_samples.emplace(bank, std::move(samples));
    m_tmp_samples = std::move(samples);
}

s16_output synth::tick()
{
    s16_output out {};
    for (auto& v : m_voices) {
        out += v->run();
    }

    // clean up disused ones?
    out.left *= 0.5;
    out.right *= 0.5;

    auto prev = m_voices.before_begin();

    m_voices.remove_if([](std::unique_ptr<voice>& v) { return v->dead(); });

    return out;
}

static std::pair<s16, s16> pitchbend(Tone& tone, int current_pb, int current_pm, int start_note, int start_fine)
{
    auto v9 = (start_note << 7) + start_fine * current_pm;
    u32 v7;
    if (current_pb >= 0)
        v7 = tone.PBHigh * (current_pb << 7) / 0x7fff + v9;
    else
        v7 = tone.PBLow * (current_pb << 7) / 0x7fff + v9;
    return { v7 / 128, v7 % 128 };
}

void synth::key_on(Tone& tone, u8 channel, u8 note, vol_pair volume)
{
    auto v = std::make_unique<voice>((u16*)(m_tmp_samples.get() + tone.VAGInSR), channel);

    v->set_volume(volume.left >> 1, volume.right >> 1);

    // TODO pb/pm function
    // auto pitch = PS1Note2Pitch(tone.CenterNote, tone.CenterFine, notes.first, notes.second);
    // auto notes = pitchbend(tone, 0, 0, note, 0);
    auto pitch = PS1Note2Pitch(tone.CenterNote, tone.CenterFine, note, 0);
    v->set_pitch(pitch);
    v->set_asdr1(tone.ADSR1);
    v->set_asdr2(tone.ADSR2);
    v->key_on();
    m_voices.emplace_front(std::move(v));
}

void synth::key_off(u8 channel, u8 note, u8 velocity)
{
    for (auto& v : m_voices) {
        if (v->m_channel == channel) {
            v->key_off();
        }
    }
}
