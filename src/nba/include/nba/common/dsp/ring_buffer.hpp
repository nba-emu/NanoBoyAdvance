/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>
#include <nba/common/dsp/stereo.hpp>
#include <nba/common/dsp/stream.hpp>

namespace nba {

template<typename T>
struct RingBuffer : Stream<T> {
  RingBuffer(int length, bool blocking = false)
      : length(length)
      , blocking(blocking) {
    data = std::make_unique<T[]>(length);
    Reset();
  }

  auto Available() -> int { return count; }

  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count  = 0;
    for(int i = 0; i < length; i++) {
      data[i] = {};
    }
  }

  auto Peek(int offset) -> T const {
    return data[(rd_ptr + offset) % length];
  }

  auto Read() -> T {
    T value = data[rd_ptr];
    if(count > 0) {
      rd_ptr = (rd_ptr + 1) % length;
      count--;
    }
    return value;
  }

  void Write(T const& value) {
    if(blocking && count == length) {
      return;
    }
    data[wr_ptr] = value;
    wr_ptr = (wr_ptr + 1) % length;
    count++;
  }

private:
  std::unique_ptr<T[]> data;

  int rd_ptr;
  int wr_ptr;
  int length;
  int count;
  bool blocking;
};

template <typename T>
using StereoRingBuffer = RingBuffer<StereoSample<T>>;

} // namespace nba
