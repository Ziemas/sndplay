#pragma once
#include "types.h"
#include <memory>

class synth {
public:
    synth(u8* sample_buffer)
        : m_sample_buffer(sample_buffer)
    {
    }

private:
    u8* m_sample_buffer { nullptr };
};
