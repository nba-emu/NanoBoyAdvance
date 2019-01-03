/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance {
namespace GBA {

struct DisplayControl {
    int mode;
    int cgb_mode;
    int frame;
    int hblank_oam_access;
    int oam_mapping_1d;
    int forced_blank;
    int enable[8];

    void Reset();
    auto Read(int address) -> std::uint8_t;
    void Write(int address, std::uint8_t value);
};

} // namespace GBA
} // namespace NanoboyAdvance
