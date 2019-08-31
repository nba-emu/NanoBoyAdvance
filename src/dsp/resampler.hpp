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

// TODO: is a shared_ptr really best fit here?

#include <cmath>
#include <memory>

#include "ring_buffer.hpp"
#include "stream.hpp"

namespace DSP {

template <typename T>
class Resampler : public Stream<T> {
public:
  Resampler(std::shared_ptr<WriteStream<T>> output) : output(output) {}
  
  auto Read() -> T { return output->Read(); }
  
  void SetSampleRates(float samplerate_in, float samplerate_out) {
    resample_phase_shift = samplerate_in / samplerate_out;
  }

protected:
  std::shared_ptr<WriteStream<T>> output;
  
  float resample_phase_shift = 1;
};

template <int points>
class SincResampler : public Resampler<float> {
public:
  void Write(float input) {
    taps.Write(input); // TODO: initial write position?

    while (resample_phase < 1.0) { 
      float sample = 0.0;

      for (int n = 0; n < points; n++) {
        float x = M_PI * (resample_phase - (n - points/2)) + 1e-6;
        float sinc = std::sin(x)/x;

        sample += sinc * taps.Peek(n);
      }

      output->Write(sample);

      resample_phase += resample_phase_shift;
    }

    taps.Read(); // advance read pointer

    resample_phase = resample_phase - 1.0;
  }
  
private:
  float resample_phase = 0; // internal phase
  
  RingBuffer<float> taps { points };
};
  
}