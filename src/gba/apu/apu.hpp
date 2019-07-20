/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include "regs.hpp"
#include "../cpu/event_device.hpp"

#include <cstdio>

namespace GameBoyAdvance {

class CPU;

class APU : public EventDevice {
public:
    APU(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    void LatchFIFO(int id, int times);
    void Tick() final;
    
    struct MMIO {
        FIFO fifo[2];
        
        SoundControl soundcnt { fifo };
    } mmio;
    
    std::int8_t latch[2];
    
private:
    CPU* cpu;
    
    FILE* dump;
};

}