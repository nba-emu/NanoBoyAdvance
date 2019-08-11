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

#include <math.h>

#include "apu.hpp"
#include "../cpu/cpu.hpp"

using namespace GameBoyAdvance;

void APU::Reset() {
  mmio.fifo[0].Reset();
  mmio.fifo[1].Reset();
  mmio.soundcnt.Reset();
  mmio.bias.Reset();
  
  dump = fopen("audio.raw", "wb");
  wait_cycles = 512;
}
  
void APU::LatchFIFO(int id, int times) {
  auto& fifo = mmio.fifo[id];
  
  for (int time = 0; time < times; time++) {
    latch[id] = fifo.Read();
    if (fifo.Count() <= 16) {
      cpu->dma.RequestFIFO(id);
    }
  }
}

void APU::Tick() {
  wait_cycles += 512;
}