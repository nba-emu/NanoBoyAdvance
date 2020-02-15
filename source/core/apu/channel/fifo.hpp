/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace GameBoyAdvance {

class FIFO {   
public:
  FIFO() { Reset(); }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count = 0;
  }
  
  int Count() const { return count; }
  
  void Write(std::int8_t sample) {
    if (count < s_fifo_len) {
      data[wr_ptr] = sample;
      wr_ptr = (wr_ptr + 1) % s_fifo_len;
      count++;
    }
  }
  
  auto Read() -> std::int8_t {
    std::int8_t value = 0;
    
    if (count > 0) {
      value = data[rd_ptr];
      rd_ptr = (rd_ptr + 1) % s_fifo_len;
      count--;
    }
    
    return value;
  }

private:
  static constexpr int s_fifo_len = 32;
  
  std::int8_t data[s_fifo_len];
 
  int rd_ptr;
  int wr_ptr;
  int count;
};
  
}
  