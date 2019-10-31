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
#include "../ring_buffer.hpp"

namespace DSP {

template <typename T, int points>
class SincResampler : public Resampler<T> {
public:
  static_assert((points % 4) == 0, "DSP::SincResampler<T, points>: points must be divisible by four.");

  SincResampler(std::shared_ptr<WriteStream<T>> output) 
    : Resampler<T>(output)
  {
    SetSampleRates(1, 1);

    for (int i = 0; i < points - 1; i++) {
      taps.Write({});
    }
  }
  
  void SetSampleRates(float samplerate_in, float samplerate_out) final {
    Resampler<T>::SetSampleRates(samplerate_in, samplerate_out);
    
    float kernelSum = 0.0;
    float cutoff = 0.95;//0.65; // TODO: do not hardcode this.
    
    if (this->resample_phase_shift > 1.0) {
      cutoff /= this->resample_phase_shift;
    }

    for (int n = 0; n < points; n++) {
      for (int m = 0; m < s_lut_resolution; m++) {
        double t  = m/double(s_lut_resolution);
        double x1 = M_PI * (t - n + points/2) + 1e-6;
        double x2 = 2 * M_PI * (n + t)/points; 
        double sinc = std::sin(cutoff * x1)/x1;
        double blackman = 0.42 - 0.49 * std::cos(x2) + 0.076 * std::cos(2 * x2);
        
        lut[n*s_lut_resolution+m] = sinc * blackman;
        kernelSum += sinc * blackman;
      }
    }
    
    kernelSum /= s_lut_resolution;
    
    for (int i = 0; i < points * s_lut_resolution; i++) {
      lut[i] /= kernelSum;
    }
  }

  void Write(T const& input) final {
    taps.Write(input);

    while (resample_phase < 1.0) { 
      T sample = {};

      int x = int(std::round(resample_phase * s_lut_resolution));
      
      for (int n = 0; n < points; n += 4) {        
        sample += taps.Peek(n+0)*lut[x+0*s_lut_resolution] +
                  taps.Peek(n+1)*lut[x+1*s_lut_resolution] + 
                  taps.Peek(n+2)*lut[x+2*s_lut_resolution] + 
                  taps.Peek(n+3)*lut[x+3*s_lut_resolution];
        
        x += 4 * s_lut_resolution;
      }

      this->output->Write(sample);

      resample_phase += this->resample_phase_shift;
    }

    taps.Read();

    resample_phase = resample_phase - 1.0;
  }
  
private:
  static constexpr int s_lut_resolution = 512;

  double lut[points * s_lut_resolution];
  float resample_phase = 0;
  RingBuffer<T> taps { points };
};

template <typename T, int points>
using SincStereoResampler = SincResampler<StereoSample<T>, points>;

}