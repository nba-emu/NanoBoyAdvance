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

namespace GameBoyAdvance {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };
  
void CPU::ResetTimers() {
  for (int id = 0; id < 4; id++) {
    mmio.timer[id].id = id;
    mmio.timer[id].cycles = 0;
    mmio.timer[id].reload = 0;
    mmio.timer[id].counter = 0;
    mmio.timer[id].control.frequency = 0;
    mmio.timer[id].control.cascade = false;
    mmio.timer[id].control.interrupt = false;
    mmio.timer[id].control.enable = false;
    mmio.timer[id].overflow = false;
    mmio.timer[id].shift = 0;
    mmio.timer[id].mask  = 0;
  }
}

auto CPU::ReadTimer(int id, int offset) -> std::uint8_t {
  auto& timer = mmio.timer[id];
  
  switch (offset) {
    case 0: {
      return timer.counter & 0xFF;
    }
    case 1: {
      return timer.counter >> 8;
    }
    case 2: {
      return (timer.control.frequency) |
             (timer.control.cascade   ? 4   : 0) |
             (timer.control.interrupt ? 64  : 0) |
             (timer.control.enable    ? 128 : 0);
    }
    default: return 0;
  }
}

void CPU::WriteTimer(int id, int offset, std::uint8_t value) {
  auto& timer = mmio.timer[id];
  auto& control = timer.control;
  
  switch (offset) {
    case 0: timer.reload = (timer.reload & 0xFF00) | (value << 0); break;
    case 1: timer.reload = (timer.reload & 0x00FF) | (value << 8); break;
    case 2: {
      bool enable_previous = control.enable;

      control.frequency = value & 3;
      control.cascade   = value & 4;
      control.interrupt = value & 64;
      control.enable    = value & 128;

      timer.shift = g_ticks_shift[control.frequency];
      timer.mask  = g_ticks_mask[control.frequency];
      
      if (!enable_previous && control.enable) {
        /* NOTE: the timing calibration test seems to
         * expect the timer to start 2 cycles later than we do.
         * We account for this here, however I'm not 100% sure if it
         * holds for the general case.
         */
        timer.cycles  = -2;
        timer.counter = timer.reload;
      }
    }
  }
}

}