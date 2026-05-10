// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/common/dsp/stereo.hh>
#include <nba/common/dsp/stream.hh>
#include <memory>

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
