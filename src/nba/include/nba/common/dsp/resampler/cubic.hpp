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
struct CubicResampler : Resampler<T> {
  CubicResampler(std::shared_ptr<WriteStream<T>> output) 
      : Resampler<T>(output) {
  }
  
  void Write(T const& input) final {
    while(resample_phase < 1.0) {
      // http://paulbourke.net/miscellaneous/interpolation/
      T a0, a1, a2, a3;
      float mu, mu2;
      
      mu  = resample_phase;
      mu2 = mu * mu;
      a0 = input - previous[0] - previous[2] + previous[1];
      a1 = previous[2] - previous[1] - a0;
      a2 = previous[0] - previous[2];
      a3 = previous[1];
      
      this->output->Write(a0*mu*mu2 + a1*mu2 + a2*mu + a3);
      
      resample_phase += this->resample_phase_shift;
    }
    
    resample_phase = resample_phase - 1.0;
    
    previous[2] = previous[1];
    previous[1] = previous[0];
    previous[0] = input;
  }
  
private:
  T previous[3] = {{},{},{}};
  float resample_phase = 0;
};

template <typename T>
using CubicStereoResampler = CubicResampler<StereoSample<T>>;

} // namespace nba
