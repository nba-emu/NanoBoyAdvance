/*
 * Copyright (C) 2021 fleroviux
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
  if (!apu->buffer) {
    return;
  }

  int requested = byte_len / sizeof(s16) / 2;
  int available = apu->buffer->Available();

  static constexpr float kMaxAmplitude = 0.999;

  // @todo: do we still need two separate code paths here?
  /*if (available >= requested) {
    for (int x = 0; x < requested; x++) {
      auto sample = apu->buffer->Read();
      sample[0] = std::clamp(sample[0], -kMaxAmplitude, kMaxAmplitude);
      sample[1] = std::clamp(sample[1], -kMaxAmplitude, kMaxAmplitude);
      sample *= 32767.0;

      stream[x*2+0] = s16(std::round(sample.left));
      stream[x*2+1] = s16(std::round(sample.right));
    }
  } else {
    int y = 0;

    for (int x = 0; x < requested; x++) {
      auto sample = apu->buffer->Peek(y);
      sample[0] = std::clamp(sample[0], -kMaxAmplitude, kMaxAmplitude);
      sample[1] = std::clamp(sample[1], -kMaxAmplitude, kMaxAmplitude);
      sample *= 32767.0;

      if (++y >= available) y = 0;

      stream[x*2+0] = s16(std::round(sample.left));
      stream[x*2+1] = s16(std::round(sample.right));
    }
  }*/
  for (int x = 0; x < requested; x++) {
    auto sample = apu->buffer->Read();
    sample[0] = std::clamp(sample[0], -kMaxAmplitude, kMaxAmplitude);
    sample[1] = std::clamp(sample[1], -kMaxAmplitude, kMaxAmplitude);
    sample *= 32767.0;

    stream[x*2+0] = s16(std::round(sample.left));
    stream[x*2+1] = s16(std::round(sample.right));
  }

  auto& drc = apu->drc;

  drc.samples_read += requested;

  if(drc.samples_read >= 65536) { // @todo: find a decent threshold
    auto time_now = std::chrono::steady_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::microseconds>(
      time_now - drc.time_old).count();

    drc.host_sample_rate = (float)drc.samples_read / time_diff * 1000000;

    // fmt::print("host: {} Hz\n", drc.host_sample_rate);

    drc.samples_read = 0;
    drc.time_old = time_now;
  }
}

} // namespace nba::core
