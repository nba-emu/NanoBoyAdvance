/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/common/dsp/resampler.hpp>

namespace nba {

template<typename T>
struct CosineResampler : Resampler<T> {
  CosineResampler(std::shared_ptr<WriteStream<T>> output) 
      : Resampler<T>(output) {
    for(int i = 0; i < kLUTsize; i++) {
      lut[i] = (std::cos(M_PI * i / (float)(kLUTsize - 1)) + 1.0) * 0.5;
    }
  }
  
  void Write(T const& input) final {
    while(resample_phase < 1.0) {
      const float index = resample_phase * (float)(kLUTsize - 1);
      const float a0 = lut[(int)index];
      const float a1 = lut[(int)index + 1];
      const float a = a0 + (a1 - a0) * (index - int(index));

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
using CosineStereoResampler = CosineResampler<StereoSample<T>>;

} // namespace nba
