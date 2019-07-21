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

#include "cpu.hpp"
#include "timer.hpp"

using namespace GameBoyAdvance;

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void TimerController::Reset() {
  for (int id = 0; id < 4; id++) {
    timer[id].id = id;
    timer[id].cycles = 0;
    timer[id].reload = 0;
    timer[id].counter = 0;
    timer[id].control.frequency = 0;
    timer[id].control.cascade = false;
    timer[id].control.interrupt = false;
    timer[id].control.enable = false;
    timer[id].overflow = false;
    timer[id].shift = 0;
    timer[id].mask = 0;
  }
}

auto TimerController::Read(int id, int offset) -> std::uint8_t {
  switch (offset) {
    case 0: {
      return timer[id].counter & 0xFF;
    }
    case 1: {
      return timer[id].counter >> 8;
    }
    case 2: {
      return (timer[id].control.frequency) |
             (timer[id].control.cascade   ? 4   : 0) |
             (timer[id].control.interrupt ? 64  : 0) |
             (timer[id].control.enable  ? 128 : 0);
    }
    default: return 0;
  }
}

void TimerController::Write(int id, int offset, std::uint8_t value) {
  auto& control = timer[id].control;
  
  switch (offset) {
    case 0: timer[id].reload = (timer[id].reload & 0xFF00) | (value << 0); break;
    case 1: timer[id].reload = (timer[id].reload & 0x00FF) | (value << 8); break;
    case 2: {
      bool enable_previous = timer[id].control.enable;

      control.frequency = value & 3;
      control.cascade   = value & 4;
      control.interrupt = value & 64;
      control.enable  = value & 128;

      timer[id].shift = g_ticks_shift[control.frequency];
      timer[id].mask  = g_ticks_mask[control.frequency];
      
      if (!enable_previous && control.enable) {
        /* NOTE: the timing calibration test seems to
         * expect the timer to start 2 cycles later than we do.
         * We account for this here, however I'm not 100% sure if it
         * holds for the general case.
         */
        timer[id].cycles = -2;
        timer[id].counter = timer[id].reload;
      }
    }
  }
}

void TimerController::Run(int cycles) {
  bool sound_enabled = cpu->apu.mmio.soundcnt.master_enable;
  
  #pragma unroll_loop
  for (int id = 0; id < 4; id++) {
    auto& control = timer[id].control;

    if (!control.enable) {
      continue;
    }
    
    if (control.cascade) {
      if (id != 0 && timer[id - 1].overflow) {
        timer[id].overflow = false;
        if (timer[id].counter != 0xFFFF) {
          timer[id].counter++;
        } else {
          timer[id].counter  = timer[id].reload;
          timer[id].overflow = true;
          if (control.interrupt)
            cpu->mmio.irq_if |= CPU::INT_TIMER0 << id;
          if (id <= 1 && sound_enabled)
            RunFIFO(id, 1);
        }
        timer[id - 1].overflow = false;
      }
    } else {
      int available = timer[id].cycles + cycles;
      int overflows = 0;

      timer[id].overflow = false;
      
      int increments  = available >> timer[id].shift;
      int to_overflow = 0x10000 - timer[id].counter;
      
      if (increments >= to_overflow) {
        timer[id].counter  = timer[id].reload;
        timer[id].overflow = true;
        overflows++;
        increments -= to_overflow;
        
        to_overflow = 0x10000 - timer[id].reload;
        if (increments >= to_overflow) {
          overflows += increments / to_overflow;
          increments %= to_overflow;
        }
      }
      
      timer[id].counter += increments;
      available &= timer[id].mask;
      
      if (timer[id].overflow) {
        if (control.interrupt)
          cpu->mmio.irq_if |= CPU::INT_TIMER0 << id;
        if (id <= 1 && sound_enabled)
          RunFIFO(id, overflows);
      }
      timer[id].cycles = available;
    }
  }
}

void TimerController::RunFIFO(int timer_id, int times) {
  auto& apu = cpu->apu;
  auto& soundcnt = apu.mmio.soundcnt;
  
  for (int fifo = 0; fifo < 2; fifo++) {
    if (soundcnt.dma[fifo].timer_id == timer_id)
      apu.LatchFIFO(fifo, times);
  }
}