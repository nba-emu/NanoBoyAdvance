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
#include <stdexcept>

#include "stream.hpp"

namespace DSP {

template <typename T>
struct StereoSample {
  T left  {};
  T right {};
  
  template <typename U>
  operator StereoSample<U>() {
    return { (U)left, (U)right };
  }
  
  T& operator[](int index) {
    if (index == 0) return left;
    if (index == 1) return right;
    
    throw std::runtime_error("DSP::StereoSample<T>: bad index for operator[].");
  }
  
  StereoSample<T> operator+(T scalar) const {
    return { left + scalar, right + scalar };
  }
  
  StereoSample<T> operator+(StereoSample<T> const& other) const {
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
  
  StereoSample<T> operator-(T scalar) const {
    return { left - scalar, right - scalar };
  }
  
  StereoSample<T> operator-(StereoSample<T> const& other) const {
    return { left - other.left, right - other.right };
  }
  
  StereoSample<T>& operator-=(T scalar) {
    left  -= scalar;
    right -= scalar;
    return *this;
  }
  
  StereoSample<T>& operator-=(StereoSample<T> const& other) {
    left  -= other.left;
    right -= other.right;
    return *this;
  }
  
  StereoSample<T> operator*(T scalar) const {
    return { left * scalar, right * scalar };
  }
  
  StereoSample<T> operator*(StereoSample<T> const& other) const {
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
  
}
