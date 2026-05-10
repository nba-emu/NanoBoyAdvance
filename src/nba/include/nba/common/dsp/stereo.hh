// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/common/dsp/stream.hh>
#include <stdexcept>

namespace nba {

template<typename T>
struct StereoSample {
  T left  {};
  T right {};

  template <typename U>
  operator StereoSample<U>() {
    return { (U)left, (U)right };
  }

  T& operator[](int index) {
    if(index == 0) return left;
    if(index == 1) return right;

    throw std::runtime_error("StereoSample<T>: bad index for operator[].");
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

} // namespace nba
