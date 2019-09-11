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

#include <bitset>
#include <cstdint>

namespace GameBoyAdvance {

class CPU;
  
class DMAx {

public:
  DMAx(CPU* cpu) : cpu(cpu) { Reset(); }
  
  enum class Occasion {
    HBlank,
    VBlank,
    FIFO0,
    FIFO1
  };
  
  void Reset();
  void Request(Occasion occasion);
  void Run(int const& ticks_left);
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);
  bool IsRunning() { return runnable.any(); }
  
private:
  void TryStart(int chan_id);
  bool TransferLoop16(int const& ticks_left);
  bool TransferLoop32(int const& ticks_left);
  void TransferFIFO();
  
  CPU* cpu;
  
  int  current;
  bool interleaved;
  
  std::bitset<4> hblank_set;
  std::bitset<4> vblank_set;
  std::bitset<4> runnable;
  
  struct Channel {
    enum Control  {
      INCREMENT = 0,
      DECREMENT = 1,
      FIXED  = 2,
      RELOAD = 3
    };

    enum Timing {
      IMMEDIATE = 0,
      VBLANK  = 1,
      HBLANK  = 2,
      SPECIAL = 3
    };

    enum Size {
      HWORD = 0,
      WORD  = 1
    };

    bool enable;
    bool repeat;
    bool interrupt;
    bool gamepak;

    std::uint16_t length;
    std::uint32_t dst_addr;
    std::uint32_t src_addr;
    Control dst_cntl;
    Control src_cntl;
    Timing time;
    Size size;

    int fifo_request_count;
    
    struct {
      std::uint32_t length;
      std::uint32_t dst_addr;
      std::uint32_t src_addr;
    } latch;
  } channels[4];
};

}