// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "ame_handler.h"

ame_handler::ame_handler(MultiMIDIBlockHeader* block, synth& synth, s32 vol, s32 pan, s8 repeats, locator& loc)
    : m_header(block)
    , m_locator(loc)
    , m_synth(synth)
    , m_vol(vol)
    , m_pan(pan)
    , m_repeats(repeats)
{
    auto firstblock = (MIDIBlockHeader*)(block->BlockPtr[0] + (uintptr_t)block);

    m_midis.emplace_front(std::make_unique<midi_handler>(firstblock, synth, vol, pan, repeats, loc, this));
};

bool ame_handler::tick()
{
    for (auto& m : m_midis) {
        m->tick();
    }

    m_midis.remove_if([](std::unique_ptr<midi_handler>& m) { return m->complete(); });

    return false;
};

void ame_handler::start_segment(u32 id)
{
    auto midiblock = (MIDIBlockHeader*)(m_header->BlockPtr[id] + (uintptr_t)m_header);
    fmt::print("starting segment {}\n", id);
    m_midis.emplace_front(std::make_unique<midi_handler>(midiblock, m_synth, m_vol, m_pan, m_repeats, m_locator, this));
}

void ame_handler::stop_segment(u32 id)
{
    fmt::print("stopping segment {}\n", id);
}

#define AME_BEGIN(op)                      \
    fmt::print("ame trace: {:x}\n", (op)); \
    if (cmp_fail) {                        \
        if (cmp_fail == 1) {               \
            cmp_fail = 0;                  \
        }                                  \
    } else                                 \
        do {

#define AME_END(x) \
    }              \
    while (0)      \
        ;          \
    stream += (x);

std::pair<bool, u8*> ame_handler::run_ame(midi_handler& midi, u8* stream)
{
    int cmp_fail = 0;
    bool done = false;
    bool cont = true;

    while (!done) {
        // auto op = static_cast<ame_op>(*stream++);
        auto op = static_cast<u8>(*stream++);
        switch (op) {
        case 0x0: {
            AME_BEGIN(op)
            if (m_excite <= (stream[0] + 1)) {
                cmp_fail = 1;
            }
            AME_END(1)
        } break;
        case 0x1: {
            AME_BEGIN(op)
            if (m_excite != (stream[0] + 1)) {
                cmp_fail = 1;
            }
            AME_END(1)
        } break;
        case 0x2: {
            AME_BEGIN(op)
            if (m_excite > (stream[0] + 1)) {
                cmp_fail = 1;
            }
            AME_END(1)
        } break;
        case 0x4: {
            if (cmp_fail == 1) {
                cmp_fail = 2;
            }
        } break;
        case 0x5: {
            if (cmp_fail == 2) {
                cmp_fail = 0;
            }
        } break;
        case 0x6: {
            AME_BEGIN(op)
            if (m_register[stream[0]] > (stream[1] - 1)) {
                cmp_fail = 1;
            }
            AME_END(2)
        } break;
        case 0x7: {
            AME_BEGIN(op)
            if (m_register[stream[0]] < (stream[1] + 1)) {
                cmp_fail = 1;
            }
            AME_END(2)
        } break;
        case 0xB: {
            m_macro[stream[0]] = &stream[1];
            while (*stream != 0xf7) {
                stream++;
            }
            stream++;
        } break;
        case 0xc: {
            AME_BEGIN(op)
            auto [sub_cont, ptr] = run_ame(midi, &stream[0]);
            if (!sub_cont) {
                cont = false;
                done = true;
            }
            AME_END(1)
        } break;
        case 0xd: {
            AME_BEGIN(op)
            cont = false;
            done = true;
            start_segment(m_register[stream[0] - 1]);
            AME_END(1)
        } break;
        case 0xe: {
            AME_BEGIN(op)
            start_segment(m_register[stream[0] - 1]);
            AME_END(1)
        } break;
        case 0xf: {
            if (cmp_fail) {
                while (*stream != 0x7f) {
                    stream++;
                }
                stream++;
                if (cmp_fail == 1)
                    cmp_fail = 0;
            } else {
                auto group = *stream++;
                fmt::print("getting groupinfo for {}\n", group);
                m_groups[group].basis = *stream++;
                u8 channel = 0;
                while (*stream != 0xf7) {
                    fmt::print("group {} channel {}:{}\n", group, channel, *stream);
                    m_groups[group].channel[channel] = *stream++;
                    fmt::print("group {} channel {} min {}\n", group, channel, *stream);
                    m_groups[group].excite_min[channel] = *stream++;
                    fmt::print("group {} channel {} max {}\n", group, channel, *stream);
                    m_groups[group].excite_max[channel] = *stream++;
                    channel++;
                }
                m_groups[group].num_channels = channel;
                stream++;
            }
        } break;
        case 0x10: {
            AME_BEGIN(op)
            u8 group = stream[0];
            // fmt::print("setting group {}\n", group);
            u8 comp = 0;
            if (m_groups[group].basis == 0) {
                comp = m_excite;
            } else {
                comp = m_register[m_groups[group].basis - 1];
                // fmt::print("reg[{}] {}\n", m_groups[group].basis - 1, comp);
            }
            for (int i = 0; i < m_groups[group].num_channels; i++) {
                // fmt::print("channel setup {} min{} cmp{} max{}\n", i, m_groups[group].excite_min[i], comp, m_groups[group].excite_max[i]);
                if ((m_groups[group].excite_min[i] - 1 >= comp) || (m_groups[group].excite_max[i] + 1 <= comp)) {
                    midi.mute_channel(m_groups[group].channel[i]);
                } else {
                    midi.unmute_channel(m_groups[group].channel[i]);
                }
            }
            AME_END(1)
        } break;
        case 0x11: {
            AME_BEGIN(op)
            done = true;
            cont = false;
            start_segment(stream[0]);
            AME_END(1)
        } break;
        case 0x12: {
            AME_BEGIN(op)
            start_segment(stream[0]);
            AME_END(1)
        } break;
        case 0x13: {
            AME_BEGIN(op)
            m_register[stream[0]] = stream[1];
            AME_END(2)
        } break;
        case 0x14: {
            AME_BEGIN(op)
            if (m_register[stream[0]] < 0x7f) {
                m_register[stream[0]]++;
            }
            AME_END(1)
        } break;
        case 0x15: {
            AME_BEGIN(op)
            if (m_register[stream[0]] > 0) {
                m_register[stream[0]]--;
            }
            AME_END(1)
        } break;
        case 0x16: {
            AME_BEGIN(op)
            if (m_register[stream[0]] != stream[1]) {
                cmp_fail = 1;
            }
            AME_END(2)
        } break;
        default: {
            throw ame_error(fmt::format("Unhandled AME event {:02x}", (u8)op));
        } break;
        }

        if (*stream == 0xf7) {
            fmt::print("ame done\n");
            stream++;
            done = true;
        }
    }

    return { cont, stream };
}

#undef AME_BEGIN
#undef AME_END
