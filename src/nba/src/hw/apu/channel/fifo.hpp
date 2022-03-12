/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba::core {

struct FIFO {
  FIFO() { Reset(); }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count = 0;
    pending = 0;
  }
  
  int Count() const { return count; }

  void WriteByte(uint offset, u8 value) {
    const auto shift = offset * 8;

    WriteWord((pending & ~(0x00FF << shift)) | (value << shift));
  }

  void WriteHalf(uint offset, u8 value) {
    const auto shift = offset * 8;

    WriteWord((pending & ~(0xFFFF << shift)) | (value << shift));
  }

  void WriteWord(u32 value) {
    pending = value;

    if (count < s_fifo_len) {
      data[wr_ptr] = value;
      if (++wr_ptr == s_fifo_len) wr_ptr = 0;
      count++;
    }
  }
  
  auto ReadWord() -> u32 {
    u32 value = data[rd_ptr];
    
    if (count > 0) {
      if (++rd_ptr == s_fifo_len) rd_ptr = 0;
      count--;
    }

    return value;
  }

private:
  static constexpr int s_fifo_len = 7;
  
  u32 data[s_fifo_len];
  u32 pending;

  int rd_ptr;
  int wr_ptr;
  int count;
};
  
} // namespace nba::core
