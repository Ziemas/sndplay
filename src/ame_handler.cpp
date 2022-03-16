// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "ame_handler.h"

bool tick() { return true; };

#define AME_OP(op, body, argc)              \
    case ame_op::op: {                      \
        fmt::print("ame_trace: {}\n", #op); \
        { body } stream                     \
            += (argc);                      \
    } break;

#define AME_COND(x)      \
    if (flag == 0) {     \
        x;               \
    } else {             \
        if (flag == 1) { \
            flag = 0;    \
        }                \
    }
#define AME_CMP(x) AME_COND(if (x) { flag = 1; })

u8* ame_handler::run_ame(midi_handler& midi, u8* stream)
{
    bool done = false;

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
            AME_OP(cond_start_segment, AME_COND(/*TODO*/), 1)
            AME_OP(cond_stop_and_start, AME_COND(/*TODO*/), 1)
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

    return stream;
}

#undef AME_CMP
#undef AME_COND
#undef AME_OP
