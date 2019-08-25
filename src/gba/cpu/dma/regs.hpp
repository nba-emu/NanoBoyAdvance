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

#include <cstdint>

namespace GameBoyAdvance {
  
struct DMA {
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

  struct {
    int request_count;
      
    std::uint32_t length;
    std::uint32_t dst_addr;
    std::uint32_t src_addr;
  } internal;
};

}