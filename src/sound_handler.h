// Copyright: 2021 - 2021, Ziemas
// SPDX-License-Identifier: ISC
#pragma once

class sound_handler {
public:
    virtual ~sound_handler() = default;
    virtual void tick() = 0;

private:
};