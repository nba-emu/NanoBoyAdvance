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

#include "fifo.hpp"

#include <cstdint>

namespace GameBoyAdvance {

enum Side {
  SIDE_LEFT  = 0,
  SIDE_RIGHT = 1
};
  
enum DMANumber {
  DMA_A = 0,
  DMA_B = 1
};
  
struct SoundControl {
  SoundControl(FIFO* fifos) : fifos(fifos) { }
  
  bool master_enable;

  struct PSG {
    int  volume;    /* 0=25% 1=50% 2=100% 3=forbidden */
    int  master[2]; /* master volume L/R (0..255) */
    bool enable[2][4];
  } psg;

  struct DMA {
    int  volume; /* 0=50%, 1=100% */
    bool enable[2];
    int  timer_id;
  } dma[2];

  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);

private:
  FIFO* fifos;
};

struct BIAS {
  int level;
  int resolution;

  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);
  
  auto GetSampleInterval() -> int { return 512 >> resolution; }
  auto GetSampleRate() -> int { return 32768 << resolution; }
};

}