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

#include "../cpu.hpp"

#include <limits>

namespace GameBoyAdvance {

void CPU::RunTimers(int cycles) {
  int available;

  for (auto& timer: mmio.timer) {
    if (timer.control.enable && !timer.control.cascade) {
      available = timer.cycles + cycles;
      
      IncrementTimer(timer.id, available >> timer.shift);
      timer.cycles = available & timer.mask;
    }
  }
}

void CPU::IncrementTimer(int id, int increment) {
  auto& timer = mmio.timer[id];
  auto& soundcnt = apu.mmio.soundcnt;
  
  int overflows = 0;
  int threshold = 0x10000 - timer.counter;
  
  if (increment >= threshold) {
    overflows++;
    increment -= threshold;
    timer.counter = timer.reload;
    
    threshold = 0x10000 - timer.reload;
    
    if (increment >= threshold) {
      overflows += increment / threshold;
      increment %= threshold;
    }
  }
      
  timer.counter += increment;
    
  if (overflows > 0) {
    if (id != 3 && mmio.timer[id + 1].control.cascade) {
      IncrementTimer(id + 1, overflows);
    }
    
    if (timer.control.interrupt) {
      mmio.irq_if |= CPU::INT_TIMER0 << id;
    }
    
    if (id <= 1 && soundcnt.master_enable) {
      for (int fifo = 0; fifo < 2; fifo++) {
        if (soundcnt.dma[fifo].timer_id == id) {
          apu.LatchFIFO(fifo, overflows);
        }
      }
    }
  }
}

auto CPU::GetCyclesToTimerIRQ() -> int {
  int cycles = std::numeric_limits<int>::max();
  
  for (auto& timer : mmio.timer) {
    if (timer.control.interrupt && 
        timer.control.enable &&
       !timer.control.cascade) 
    {
      int required = ((0x10000 - timer.counter) << timer.shift) - timer.cycles;
      
      if (required < cycles) {
        cycles = required;
      }
    }
  }
  
  return cycles;
}

}
