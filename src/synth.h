// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include "types.h"
#include <memory>

class synth {
public:
    synth(u8* sample_buffer)
        : m_sample_buffer(sample_buffer)
    {
    }

    void run(u32 samples, u8 *output);
private:
    u8* m_sample_buffer { nullptr };
};
