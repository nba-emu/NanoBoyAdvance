/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/common/dsp/resampler.hpp>
#include <nba/common/dsp/ring_buffer.hpp>

namespace nba {

template<typename T, int points>
struct SincResampler : Resampler<T> {
  static_assert((points % 4) == 0, "SincResampler<T, points>: points must be divisible by four.");

  SincResampler(std::shared_ptr<WriteStream<T>> output) 
      : Resampler<T>(output) {
    SetSampleRates(1, 1);

    for(int i = 0; i < points - 1; i++) {
      taps.Write({});
    }
  }
  
  void SetSampleRates(float samplerate_in, float samplerate_out) final {
    Resampler<T>::SetSampleRates(samplerate_in, samplerate_out);
    
    float kernelSum = 0.0;
    float cutoff = 0.9;
    
    if(this->resample_phase_shift > 1.0) {
      cutoff /= this->resample_phase_shift;
    }

    for(int n = 0; n < points; n++) {
      for(int m = 0; m < s_lut_resolution; m++) {
        double t  = m/double(s_lut_resolution);
        double x1 = M_PI * (t - n + points/2) + 1e-6;
        double x2 = 2 * M_PI * (n + t)/points; 
        double sinc = std::sin(cutoff * x1)/x1;
        double blackman = 0.42 - 0.49 * std::cos(x2) + 0.076 * std::cos(2 * x2);
        
        lut[n * s_lut_resolution + m] = sinc * blackman;
        kernelSum += sinc * blackman;
      }
    }
    
    kernelSum /= s_lut_resolution;
    
    for(int i = 0; i < points * s_lut_resolution; i++) {
      lut[i] /= kernelSum;
    }
  }

  void Write(T const& input) final {
    taps.Write(input);

    while(resample_phase < 1.0) { 
      T sample = {};

      int x = (int)(resample_phase * s_lut_resolution);
      
      for(int n = 0; n < points; n += 4) {        
        sample += taps.Peek(n + 0) * lut[x + 0 * s_lut_resolution];
        sample += taps.Peek(n + 1) * lut[x + 1 * s_lut_resolution]; 
        sample += taps.Peek(n + 2) * lut[x + 2 * s_lut_resolution]; 
        sample += taps.Peek(n + 3) * lut[x + 3 * s_lut_resolution];
        
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

} // namespace nba
