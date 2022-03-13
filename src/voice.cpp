
// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#include "voice.h"
#include "types.h"
#include <array>

#include "interp_table.inc"

// Integer math version of ps-adpcm coefs
static constexpr std::array<std::array<s16, 2>, 5> adpcm_coefs = { {
    { 0, 0 },
    { 60, 0 },
    { 115, -52 },
    { 98, -55 },
    { 122, -60 },
} };

void voice::run()
{

}
