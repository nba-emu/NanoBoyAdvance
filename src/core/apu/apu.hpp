/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "regs.hpp"

namespace NanoboyAdvance::GBA {

class APU {
public:
    void Reset() {
        mmio.soundcnt.Reset();
    }
    
    struct MMIO {
        SoundControl soundcnt;
    } mmio;
    
private:
};

}