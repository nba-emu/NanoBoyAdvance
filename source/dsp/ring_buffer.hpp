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

#include <memory>

#include "stereo.hpp"
#include "stream.hpp"

namespace DSP {

template <typename T>
class RingBuffer : public Stream<T> {
public:
  RingBuffer(int length, bool blocking = false)
    : length(length)
    , blocking(blocking)
  {
    data.reset(new T[length]);
    Reset();
  }
  
  auto Available() -> int { return count; }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
    count  = 0;
  }
  
  auto Peek(int offset) -> T const {
    return data[(rd_ptr + offset) % length];
  }
  
  auto Read() -> T {
    T value = data[rd_ptr];
    if (!blocking || count > 0) {
      rd_ptr = (rd_ptr + 1) % length;
      count--;
    }
    return value;
  }
  
  void Write(T const& value) {
    if (blocking && count == length) {
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
  
}
