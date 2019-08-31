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

#include "stream.hpp"

namespace DSP {

template <typename T>
struct StereoSample {
  T left  {};
  T right {};
  
  // Conversion e.g. from StereoSample<int> to StereoSample<float>
  template <typename U>
  operator StereoSample<U>() {
    return { (U)left, (U)right };
  }
  
  StereoSample<T> operator+(T scalar) {
    return { left + scalar, right + scalar };
  }
  
  StereoSample<T> operator+(StereoSample<T> const& other) {
    return { left + other.left, right + other.right };
  }
  
  StereoSample<T>& operator+=(T scalar) {
    left  += scalar;
    right += scalar;
    return *this;
  }
  
  StereoSample<T>& operator+=(StereoSample<T> const& other) {
    left  += other.left;
    right += other.right;
    return *this;
  }
  
  StereoSample<T> operator*(T scalar) {
    return { left * scalar, right * scalar };
  }
  
  StereoSample<T> operator*(StereoSample<T> const& other) {
    return { left * other.left, right * other.right };
  }
  
  StereoSample<T>& operator*=(T scalar) {
    left  *= scalar;
    right *= scalar;
    return *this;
  }
  
  StereoSample<T>& operator*=(StereoSample<T> const& other) {
    left  *= other.left;
    right *= other.right;
    return *this;
  }
};

template <typename T>
class RingBuffer : public Stream<T> {

public:
  RingBuffer(int length)
    : length(length)
  {
    data.reset(new T[length]);
    Reset();
  }
  
  void Reset() {
    rd_ptr = 0;
    wr_ptr = 0;
  }
  
  auto Peek(int offset) -> T const {
    return data[(rd_ptr + offset) % length];
  }
  
  auto Read() -> T {
    T value = data[rd_ptr];
    rd_ptr = (rd_ptr + 1) % length;
    return value;
  }
  
  void Write(T const& value) {
    data[wr_ptr] = value;
    wr_ptr = (wr_ptr + 1) % length;
  }
  
private:
  std::unique_ptr<T[]> data;
  
  int rd_ptr;
  int wr_ptr;
  int length;
};

template <typename T>
using StereoRingBuffer = RingBuffer<StereoSample<T>>;
  
}
