/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cmath>
#include <memory>

#ifndef M_PI
#define M_PI (3.141592653589793238463)
#endif

#include "stereo.hpp"
#include "stream.hpp"

namespace common::dsp {

template <typename T>
struct Resampler : WriteStream<T> {
  Resampler(std::shared_ptr<WriteStream<T>> output) : output(output) {}
  
  virtual void SetSampleRates(float samplerate_in, float samplerate_out) {
    resample_phase_shift = samplerate_in / samplerate_out;
  }

protected:
  std::shared_ptr<WriteStream<T>> output;
  
  float resample_phase_shift = 1;
};

template <typename T>
using StereoResampler = Resampler<StereoSample<T>>;

} // namespace common::dsp
