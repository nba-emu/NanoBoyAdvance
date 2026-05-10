// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>

#include "hw/apu/channel/quad_channel.hh"
#include "hw/apu/channel/wave_channel.hh"
#include "hw/apu/channel/noise_channel.hh"
#include "hw/apu/channel/fifo.hh"

namespace nba::core {

enum Side {
  SIDE_LEFT  = 0,
  SIDE_RIGHT = 1
};

enum DMANumber {
  DMA_A = 0,
  DMA_B = 1
};

struct SoundControl {
  SoundControl(
    FIFO* fifos,
    QuadChannel& psg1,
    QuadChannel& psg2,
    WaveChannel& psg3,
    NoiseChannel& psg4)
      : fifos(fifos)
      , psg1(psg1)
      , psg2(psg2)
      , psg3(psg3)
      , psg4(psg4) {
  }

  bool master_enable;

  struct PSG {
    int  volume;
    int  master[2];
    bool enable[2][4];
  } psg;

  struct DMA {
    int  volume;
    bool enable[2];
    int  timer_id;
  } dma[2];

  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadWord() -> u32;
  void WriteWord(u32 value);

private:
  FIFO* fifos;

  QuadChannel& psg1;
  QuadChannel& psg2;
  WaveChannel& psg3;
  NoiseChannel& psg4;
};

struct BIAS {
  int level;
  int resolution;

  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);

  auto GetSampleInterval() -> int { return 512 >> resolution; }
  auto GetSampleRate() -> int { return 32768 << resolution; }
};

} // namespace nba::core
