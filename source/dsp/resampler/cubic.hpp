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

#include "../resampler.hpp"

namespace DSP {

template <typename T>
class CubicResampler : public Resampler<T> {
public:
  CubicResampler(std::shared_ptr<WriteStream<T>> output) 
    : Resampler<T>(output)
  { }
  
  void Write(T const& input) final {
    while (resample_phase < 1.0) {
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

}