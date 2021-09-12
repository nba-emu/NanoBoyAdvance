/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba::core {

class FIFO {   
public:
  FIFO() { Reset(); }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count = 0;
  }
  
  int Count() const { return count; }
  
  void Write(s8 sample) {
    if (count < s_fifo_len) {
      data[wr_ptr] = sample;
      wr_ptr = (wr_ptr + 1) % s_fifo_len;
      count++;
    }
  }
  
  auto Read() -> s8 {
    s8 value = data[rd_ptr];
    
    if (count > 0) {
      rd_ptr = (rd_ptr + 1) % s_fifo_len;
      count--;
    }
    
    return value;
  }

private:
  static constexpr int s_fifo_len = 32;
  
  s8 data[s_fifo_len];
 
  int rd_ptr;
  int wr_ptr;
  int count;
};
  
} // namespace nba::core
  