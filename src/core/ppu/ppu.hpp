/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "io.hpp"

namespace NanoboyAdvance {
namespace GBA {

class PPU {

public:
    struct MMIO {
        DisplayControl dispcnt;
        DisplayStatus dispstat;
    } mmio;

    void Reset() {
        mmio.dispcnt.Reset();
        mmio.dispstat.Reset();
    }
};

} // namespace GBA
} // namespace NanoboyAdvance
