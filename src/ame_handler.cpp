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

    // TODO
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
}

#define AME_OP(op, body, argc)              \
    case ame_op::op: {                      \
        fmt::print("ame_trace: {}\n", #op); \
        { body } stream                     \
            += (argc);                      \
    } break;

#define AME_COND(x)      \
    if (flag == 0) {     \
        do {             \
            x;           \
        } while (0);     \
    } else {             \
        if (flag == 1) { \
            flag = 0;    \
        }                \
    }
#define AME_CMP(x) AME_COND(if (x) { flag = 1; })

std::pair<bool, u8*> ame_handler::run_ame(midi_handler& midi, u8* stream)
{
    bool done = false;
    bool cont = true;

    // don't really understand this yet
    int flag = 0;

    while (!done) {
        auto op = static_cast<ame_op>(*stream++);
        switch (op) {
            AME_OP(cmp_excite_less, AME_CMP(m_excite < stream[0]), 1)
            AME_OP(cmp_excite_not_equal, AME_CMP(m_excite != stream[0]), 1)
            AME_OP(cmp_excite_greater, AME_CMP(m_excite > stream[0]), 1)
            AME_OP(cond_stop_stream, AME_COND(/* stop stream matching ident */), 1)
            AME_OP(comp_midireg_greater, AME_CMP(m_register[stream[0]] > stream[1]), 2)
            AME_OP(comp_midireg_less, AME_CMP(m_register[stream[0]] < stream[1]), 2)
            AME_OP(store_macro, m_macro[*stream] = stream; for (; *stream != 0xf7; stream++);, 1)
            AME_OP(cond_run_macro, AME_COND(run_ame(midi, m_macro[stream[0]])), 1)
            AME_OP(cond_set_reg, AME_COND(m_register[stream[0]] = stream[1]), 2)
            AME_OP(cond_start_segment, AME_COND(start_segment(stream[0])), 1)
            AME_OP(cond_stop_and_start, AME_COND(start_segment(stream[0]); cont = false;), 1)
            AME_OP(thing, AME_COND(/*TODO*/), 1)
        case ame_op::read_group_data: { // read in new group data
            if (!flag) {
                u8 group = *stream++;
                m_groups[group].basis = *stream++;
                u8 channel = 0;
                while (*stream != 0xf7) {
                    m_groups[group].channel[channel] = *stream++;
                    m_groups[group].excite_min[channel] = *stream++;
                    m_groups[group].excite_max[channel] = *stream++;
                    channel += 1;
                }
                m_groups[group].num_channels = channel;
                fmt::print("{} channels\n", channel);
                stream++;
            } else {
                for (; *stream != 0xf7; stream++)
                    ;
                flag = true;
            }
            break;
        }
        default: {
            throw ame_error(fmt::format("Unhandled AME event {:02x}", (u8)op));
        }
        }

        if (*stream == 0xf7) {
            fmt::print("ame done\n");
            done = true;
        }
    }

    return { cont, stream };
}

#undef AME_CMP
#undef AME_COND
#undef AME_OP
