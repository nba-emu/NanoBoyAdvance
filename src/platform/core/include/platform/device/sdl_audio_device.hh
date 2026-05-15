// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/device/audio_device.hh>
#include <SDL3/SDL.h>

#include <vector>

namespace nba {

struct SDL3_AudioDevice : AudioDevice {
  bool Open(void* userdata, Callback callback) final;
  void Reset() final;
  void Close() final;
  auto GetSampleRate() -> int final { return m_specification.freq; }
  void SetPause(bool value) final;

private:
  static void InternalCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

  SDL_AudioStream* m_stream = nullptr;
  SDL_AudioSpec m_specification;
  Callback m_callback;
  void* m_userdata;

  std::vector<s16> m_data;
};

} // namespace nba
