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
class CosineResampler : public Resampler<T> {
public:
  CosineResampler(std::shared_ptr<WriteStream<T>> output) 
    : Resampler<T>(output)
  {
    /* TODO: do not generate the table every time. */
    for (int i = 0; i < s_lut_resolution; i++) {
      lut[i] = (std::cos(M_PI * i/float(s_lut_resolution)) + 1.0) * 0.5;
    }
  }
  
  void Write(T const& input) final {
    while (resample_phase < 1.0) {
      float x = lut[(int)std::round(resample_phase * s_lut_resolution)];
      
      this->output->Write(previous * x + input * (1.0 - x));
      
      resample_phase += this->resample_phase_shift;
    }
    
    resample_phase = resample_phase - 1.0;
    
    previous = input;
  }
  
private:
  static constexpr int s_lut_resolution = 512;
  
  T previous = {};
  float resample_phase = 0;
  float lut[s_lut_resolution];
};

template <typename T>
using CosineStereoResampler = CosineResampler<StereoSample<T>>;

}