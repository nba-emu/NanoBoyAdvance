// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/common/dsp/resampler.hh>

namespace nba {

template<typename T>
struct NearestResampler : Resampler<T> {
  NearestResampler(std::shared_ptr<WriteStream<T>> output)
      : Resampler<T>(output) {
  }

  void Write(T const& input) final {
    while(resample_phase < 1.0) {
      this->output->Write(input);
      resample_phase += this->resample_phase_shift;
    }

    resample_phase = resample_phase - 1.0;
  }

private:
  float resample_phase = 0;
};

template <typename T>
using NearestStereoResampler = NearestResampler<StereoSample<T>>;

} // namespace nba
