/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "fifo.hpp"
#include "regs.hpp"

namespace NanoboyAdvance::GBA {

class APU {
public:
    void Reset() {
        mmio.soundcnt.Reset();
        fifo[0].Reset();
        fifo[1].Reset();
    }
    
    struct MMIO {
        SoundControl soundcnt;
    } mmio;
    
    FIFO fifo[2];
    
private:
};

}