/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "io.hpp"

namespace NanoboyAdvance {
namespace GBA {

void DisplayControl::Reset() {
    Write(0, 0);
    Write(1, 0);
}

auto DisplayControl::Read(int address) -> std::uint8_t {
    switch (address) {
    case 0:
        return mode |
            (cgb_mode << 3) |
            (frame << 4) |
            (hblank_oam_access << 5) |
            (oam_mapping_1d << 6) |
            (forced_blank << 7);
    case 1:
        return enable[0] |
            (enable[1] << 1) |
            (enable[2] << 2) |
            (enable[3] << 3) |
            (enable[4] << 4) |
            (enable[5] << 5) |
            (enable[6] << 6) |
            (enable[7] << 7);
    }
}

void DisplayControl::Write(int address, std::uint8_t value) {
    switch (address) {
    case 0:
        mode = value & 7;
        cgb_mode = (value >> 3) & 1;
        frame = (value >> 4) & 1;
        hblank_oam_access = (value >> 5) & 1;
        oam_mapping_1d = (value >> 6) & 1;
        forced_blank = (value >> 7) & 1;
        break;
    case 1:
        for (int i = 0; i < 8; i++) {
            enable[i] = (value >> i) & 1;
        }
        break;
    }
}

} // namespace GBA
} // namespace NanoboyAdvance