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
class NearestResampler : public Resampler<T> {

public:
  NearestResampler(std::shared_ptr<WriteStream<T>> output) 
    : Resampler<T>(output)
  { }
  
  void Write(T const& input) final {
    while (resample_phase < 1.0) {
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

}