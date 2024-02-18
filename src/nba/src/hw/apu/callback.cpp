/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <cmath>

#include "hw/apu/apu.hpp"

namespace nba::core {

void AudioCallback(APU* apu, s16* stream, int byte_len) {
  std::lock_guard<std::mutex> guard(apu->buffer_mutex);

  // Do not try to access the buffer if it wasn't setup yet.
  if(!apu->buffer) {
    return;
  }

  int samples = byte_len/sizeof(s16)/2;
  int available = apu->buffer->Available();

  static constexpr float kMaxAmplitude = 0.999;

  const float volume = (float)std::clamp(apu->config->audio.volume, 0, 100) / 100.0f;

  if(available >= samples) {
    for(int x = 0; x < samples; x++) {
      auto sample = apu->buffer->Read() * volume;
      sample[0] = std::clamp(sample[0], -kMaxAmplitude, kMaxAmplitude);
      sample[1] = std::clamp(sample[1], -kMaxAmplitude, kMaxAmplitude);
      sample *= 32767.0;

      stream[x*2+0] = (s16)std::round(sample.left);
      stream[x*2+1] = (s16)std::round(sample.right);
    }
  } else {
    int y = 0;

    for(int x = 0; x < samples; x++) {
      auto sample = apu->buffer->Peek(y) * volume;
      sample[0] = std::clamp(sample[0], -kMaxAmplitude, kMaxAmplitude);
      sample[1] = std::clamp(sample[1], -kMaxAmplitude, kMaxAmplitude);
      sample *= 32767.0;

      if(++y >= available) y = 0;

      stream[x*2+0] = (s16)std::round(sample.left);
      stream[x*2+1] = (s16)std::round(sample.right);
    }
  }
}

} // namespace nba::core
