// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace snd {

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

using byte = std::byte;

struct SoundBank;
class locator {
public:
    virtual ~locator() = default;
    virtual SoundBank& get_bank(u32 id) = 0;
    virtual u16* get_bank_samples(u32 id) = 0;
};

struct vol_pair {
    s16 left;
    s16 right;
};

struct LocAndSize {
    /*   0 */ u32 offset;
    /*   4 */ u32 size;
};

template <size_t chunks>
struct FileAttributes {
    /*   0 */ u32 type;
    /*   4 */ u32 num_chunks;
    /*   8 */ LocAndSize where[chunks];
};

struct s16_output {
    s16 left { 0 }, right { 0 };

    s16_output& operator+=(const s16_output& rhs)
    {
        left = static_cast<s16>(std::clamp<s32>(left + rhs.left, INT16_MIN, INT16_MAX));
        right = static_cast<s16>(std::clamp<s32>(right + rhs.right, INT16_MIN, INT16_MAX));
        return *this;
    }
};

}
