/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>
#include <nba/save_state.hpp>

namespace nba::core {

struct FIFO {
  FIFO() { Reset(); }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count = 0;

    for(u32& word : data) word = 0U;
  }
  
  int Count() const { return count; }

  void WriteByte(uint offset, u8 value) {
    const auto shift = offset * 8;

    WriteWord((data[wr_ptr] & ~(0x00FF << shift)) | (value << shift));
  }

  void WriteHalf(uint offset, u8 value) {
    const auto shift = offset * 8;

    WriteWord((data[wr_ptr] & ~(0xFFFF << shift)) | (value << shift));
  }

  void WriteWord(u32 value) {
    if(count < s_fifo_len) {
      data[wr_ptr] = value;
      if(++wr_ptr == s_fifo_len) wr_ptr = 0;
      count++;
    } else {
      Reset();
    }
  }
  
  auto ReadWord() -> u32 {
    u32 value = data[rd_ptr];
    
    if(count > 0) {
      if(++rd_ptr == s_fifo_len) rd_ptr = 0;
      count--;
    }

    return value;
  }

  void LoadState(SaveState::APU::FIFO const& state) {
    rd_ptr = 0;
    wr_ptr = state.count % s_fifo_len;
    count = state.count; 

    for(int i = 0; i < s_fifo_len; i++) {
      data[i] = state.data[i];
    }
  }

  void CopyState(SaveState::APU::FIFO& state) {
    state.count = count;

    for(int i = 0; i < s_fifo_len; i++) {
      state.data[i] = data[(rd_ptr + i) % s_fifo_len];
    }
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
