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

/* TODO: is a shared_ptr really best fit here? */

#include <cmath>
#include <memory>

#ifndef M_PI
#define M_PI (3.141592653589793238463)
#endif

#include "ring_buffer.hpp"
#include "stereo.hpp"
#include "stream.hpp"

namespace DSP {

template <typename T>
class Resampler : public WriteStream<T> {

public:
  Resampler(std::shared_ptr<WriteStream<T>> output) : output(output) {}
  
  virtual void SetSampleRates(float samplerate_in, float samplerate_out) {
    resample_phase_shift = samplerate_in / samplerate_out;
  }

protected:
  std::shared_ptr<WriteStream<T>> output;
  
  float resample_phase_shift = 1;
};

template <typename T>
using StereoResampler = Resampler<StereoSample<T>>;  

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