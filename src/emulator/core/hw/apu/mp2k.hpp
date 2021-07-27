/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/integer.hpp>
#include <common/m4a.hpp>
#include <emulator/core/arm/memory.hpp>

namespace nba::core {

// TODO: get rid of the extensive copying of e.g. M4ASoundInfo
// TODO: move definition of M4ASoundInfo into this class.

struct MP2K {
  MP2K(arm::MemoryBase& memory) : memory(memory) {
    Reset();
  }

  void Reset() {
    engaged = false;
    total_frame_count = 0;
    current_frame = 0;
    buffer_read_index = 0;
    for (auto& entry : channel_cache) {
      entry = {};
    }
  }

  bool IsEngaged() const {
    return engaged;
  }
  
  void SoundMainRAM(M4ASoundInfo sound_info);
  void RenderFrame();
  auto ReadSample() -> float*;

private:
  static constexpr int kDMABufferSize = 1582;
  static constexpr int kSampleRate = 65536;
  static constexpr int kSamplesPerFrame = kSampleRate / 60 + 1;

  // TODO: get rid of this terribleness.
  enum Status : u8 {
    CHANNEL_START = 0x80,
    CHANNEL_STOP = 0x40,
    CHANNEL_LOOP = 0x10,
    CHANNEL_IEC = 0x04,

    CHANNEL_ENV_MASK = 0x03,
    CHANNEL_ENV_ATTACK = 0x03,
    CHANNEL_ENV_DECAY = 0x02,
    CHANNEL_ENV_SUSTAIN = 0x01,
    CHANNEL_ENV_RELEASE = 0x00,
    
    CHANNEL_ON = CHANNEL_START | CHANNEL_STOP | CHANNEL_IEC | CHANNEL_ENV_MASK 
  };

  // TODO: get rid of this terribleness.
  struct {
    bool forward_loop;
    u32 sample_rate;
    u32 loop_position;
    u32 number_of_samples;
    u32 pcm_base_address;
    int current_position;

    float sample_history[4];
    float resample_phase;
    bool should_fetch_sample;
  } channel_cache[kM4AMaxDirectSoundChannels];

  bool engaged;
  arm::MemoryBase& memory;
  M4ASoundInfo sound_info;
  std::unique_ptr<float[]> buffer;
  int total_frame_count;
  int current_frame;
  int buffer_read_index;
};

} // namespace nba::core
