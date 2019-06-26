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

class CPU;

class APU {
public:
    APU(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    void LatchFIFO(int id, int times);
    
    struct MMIO {
        SoundControl soundcnt;
    } mmio;
    
    FIFO fifo[2];
    std::int8_t latch[2];
    
private:
    CPU* cpu;
    
};

}