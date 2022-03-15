#include "synth.h"
#include "player.h"

u16 NotePitchTable[] = {
    0x8000, 0x879C, 0x8FAC, 0x9837, 0xA145, 0xAADC, 0xB504,
    0xBFC8, 0xCB2F, 0xD744, 0xE411, 0xF1A1, 0x8000, 0x800E,
    0x801D, 0x802C, 0x803B, 0x804A, 0x8058, 0x8067, 0x8076,
    0x8085, 0x8094, 0x80A3, 0x80B1, 0x80C0, 0x80CF, 0x80DE,
    0x80ED, 0x80FC, 0x810B, 0x811A, 0x8129, 0x8138, 0x8146,
    0x8155, 0x8164, 0x8173, 0x8182, 0x8191, 0x81A0, 0x81AF,
    0x81BE, 0x81CD, 0x81DC, 0x81EB, 0x81FA, 0x8209, 0x8218,
    0x8227, 0x8236, 0x8245, 0x8254, 0x8263, 0x8272, 0x8282,
    0x8291, 0x82A0, 0x82AF, 0x82BE, 0x82CD, 0x82DC, 0x82EB,
    0x82FA, 0x830A, 0x8319, 0x8328, 0x8337, 0x8346, 0x8355,
    0x8364, 0x8374, 0x8383, 0x8392, 0x83A1, 0x83B0, 0x83C0,
    0x83CF, 0x83DE, 0x83ED, 0x83FD, 0x840C, 0x841B, 0x842A,
    0x843A, 0x8449, 0x8458, 0x8468, 0x8477, 0x8486, 0x8495,
    0x84A5, 0x84B4, 0x84C3, 0x84D3, 0x84E2, 0x84F1, 0x8501,
    0x8510, 0x8520, 0x852F, 0x853E, 0x854E, 0x855D, 0x856D,
    0x857C, 0x858B, 0x859B, 0x85AA, 0x85BA, 0x85C9, 0x85D9,
    0x85E8, 0x85F8, 0x8607, 0x8617, 0x8626, 0x8636, 0x8645,
    0x8655, 0x8664, 0x8674, 0x8683, 0x8693, 0x86A2, 0x86B2,
    0x86C1, 0x86D1, 0x86E0, 0x86F0, 0x8700, 0x870F, 0x871F,
    0x872E, 0x873E, 0x874E, 0x875D, 0x876D, 0x877D, 0x878C
};

static u16 sceSdNote2Pitch(u16 center_note, u16 center_fine, u16 note, short fine)
{
    s32 _fine;
    s32 _fine2;
    s32 _note;
    s32 offset1, offset2;
    s32 val;
    s32 val2;
    s32 val3;
    s32 ret;

    _fine = fine + (u16)center_fine;
    _fine2 = _fine;

    if (_fine < 0)
        _fine2 = _fine + 127;

    _fine2 = _fine2 / 128;
    _note = note + _fine2 - center_note;
    val3 = _note / 6;

    if (_note < 0)
        val3--;

    offset2 = _fine - _fine2 * 128;

    if (_note < 0)
        val2 = -1;
    else
        val2 = 0;
    if (val3 < 0)
        val3--;

    val2 = (val3 / 2) - val2;
    val = val2 - 2;
    offset1 = _note - (val2 * 12);

    if ((offset1 < 0) || ((offset1 == 0) && (offset2 < 0))) {
        offset1 = offset1 + 12;
        val = val2 - 3;
    }

    if (offset2 < 0) {
        offset1 = (offset1 - 1) + _fine2;
        offset2 += (_fine2 + 1) * 128;
    }

    ret = (NotePitchTable[offset1] * NotePitchTable[offset2 + 12]) / 0x10000;

    if (val < 0)
        ret = (ret + (1 << (-val - 1))) >> -val;

    return (u16)ret;
}

static u16 PS1Note2Pitch(char old_center, char old_fine, short new_center, short new_fine)
{
    bool thing = false;
    if (old_center >= 0) {
        thing = true;
    } else {
        thing = false;
        old_center = -old_center;
    }

    auto pitch = sceSdNote2Pitch(old_center, old_fine, new_center, new_fine);
    if (thing) {
        pitch = 44100 * pitch / 48000;
    }

    //if (-1 >= old_center) {
    //    old_center = -old_center;
    //}
    //auto pitch = sceSdNote2Pitch(old_center, old_fine, new_center, new_fine);
    //if (-1 < old_center) {
    //    pitch = (pitch * 44100) / 48000;
    //}

    return pitch;
}

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
    out.left *= 0.4;
    out.right *= 0.4;

    //auto before = m_voices.begin();
    //for (auto i = m_voices.begin(); i != m_voices.end();) {
    //    if (i->get()->dead()) {
    //        i = m_voices.erase_after(before);
    //    } else {
    //        before = i;
    //        ++i;
    //    }
    //}

    return out;
}

static std::pair<s32, s32> make_volume(int current_vol, int new_vol, int pan, int prog_vol, int prog_pan, int tone_vol, int tone_pan)
{
    s32 vol = (((((new_vol * 258 * tone_vol) / 0x7f) * prog_vol) / 0x7f) * current_vol) / 0x7f;

    // TODO rest of this function

    return { vol, vol };
}

static std::pair<s16, s16> pitchbend(Tone& tone, int current_pb, int current_pm, int start_note, int start_fine) {
    auto v9 = (start_note << 7) + start_fine * current_pm;
    u32 v7;
    if (current_pb >= 0)
        v7 = tone.PBHigh * (current_pb << 7) / 0x7fff + v9;
    else
        v7 = tone.PBLow * (current_pb << 7) / 0x7fff + v9;
    return {v7 / 128, v7 % 128};
}

void synth::key_on(Tone& tone, u8 channel, u8 note, u8 velocity, u8 vol, s16 pan)
{
    auto v = std::make_unique<voice>((u16*)(m_tmp_samples.get() + tone.VAGInSR), channel);

    auto volume = make_volume(0x7f, velocity, pan, vol, 0, tone.Vol, tone.Pan);

    v->set_volume(volume.first, volume.second);

    // TODO pb/pm function
    //auto notes = pitchbend(tone, 0, 0, note, 0);
    //auto pitch = PS1Note2Pitch(tone.CenterNote, tone.CenterFine, notes.first, notes.second);
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
