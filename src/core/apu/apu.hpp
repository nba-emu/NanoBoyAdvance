/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "regs.hpp"
#include "../event_device.hpp"

#include <cstdio>

namespace NanoboyAdvance::GBA {

class CPU;

class APU : public EventDevice {
public:
    APU(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    void LatchFIFO(int id, int times);
    bool Tick() final;
    
    struct MMIO {
        FIFO fifo[2];
        
        SoundControl soundcnt { fifo };
    } mmio;
    
    std::int8_t latch[2];
    
private:
    CPU* cpu;
    
    //FILE* dump[2];
    FILE* dump;
};

}