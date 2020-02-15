/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

#include "channel/fifo.hpp"

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