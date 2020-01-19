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

#include <cmath>
#include <memory>

#ifndef M_PI
#define M_PI (3.141592653589793238463)
#endif

#include "stereo.hpp"
#include "stream.hpp"

namespace DSP {

template <typename T>
class Resampler : public WriteStream<T> {
public:
  Resampler(std::shared_ptr<WriteStream<T>> output) : output(output) {}
  virtual ~Resampler() {}
  
  virtual void SetSampleRates(float samplerate_in, float samplerate_out) {
    resample_phase_shift = samplerate_in / samplerate_out;
  }

protected:
  /* TODO: is a shared_ptr really best fit here? */
  std::shared_ptr<WriteStream<T>> output;
  
  float resample_phase_shift = 1;
};

template <typename T>
using StereoResampler = Resampler<StereoSample<T>>;

}