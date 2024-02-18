/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cmath>
#include <memory>
#include <nba/common/dsp/stereo.hpp>
#include <nba/common/dsp/stream.hpp>

#ifndef M_PI
#define M_PI (3.141592653589793238463)
#endif

namespace nba {

template<typename T>
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

} // namespace nba
