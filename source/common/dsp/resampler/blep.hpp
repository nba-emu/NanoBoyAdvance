/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include "../resampler.hpp"

namespace common::dsp {

template <typename T>
class BlepResampler : public Resampler<T> {
public:
  BlepResampler(std::shared_ptr<WriteStream<T>> output)
    : Resampler<T>(output) {
    static constexpr int kTaylorPolyMaxIter = 5;
    static constexpr int kHalfedLUTSize = kLUTsize / 2;
    
    double scale;

    for (int i = 0; i < kLUTsize; i++) {
      double sign = -1;
      double faculty = 1;
      double x = (i - kHalfedLUTSize) / double(kHalfedLUTSize) * M_PI;
      double x_squared = x * x;
      double result = x;

      for (int j = 3; j < (2 * kTaylorPolyMaxIter + 1); j += 2) {
        x *= x_squared;
        faculty *= (j - 1) * j;
        result += sign * x / (faculty * j);
        sign = -sign;
      }

      // Normalize interpolation kernel to [0, 1] range.
      if (i == 0) {
        scale = result;
      }
      lut[i] = result * 0.5 / scale + 0.5;
    }
  }

  void Write(T const& input) final {
    while (resample_phase < 1.0) {
      auto index = resample_phase * kLUTsize;
      float a0 = lut[int(index) + 0];
      float a1 = lut[int(index) + 1];
      float a = a0 + (a1 - a0) * (index - int(index));

      this->output->Write(previous * a + input * (1.0 - a));

      resample_phase += this->resample_phase_shift;
    }

    resample_phase = resample_phase - 1.0;

    previous = input;
  }

private:
  static constexpr int kLUTsize = 512;

  T previous = {};
  float resample_phase = 0;
  float lut[kLUTsize];
};

template <typename T>
using BlepStereoResampler = BlepResampler<StereoSample<T>>;

} // namespace common::dsp
