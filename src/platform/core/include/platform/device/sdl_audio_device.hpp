/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <nba/device/audio_device.hpp>
#include <nba/log.hpp>
#include <SDL.h>

namespace nba {

struct SDL2_AudioDevice : AudioDevice {
  void SetSampleRate(int sample_rate);
  void SetBlockSize(int buffer_size);
  void SetPassthrough(SDL_AudioCallback passthrough);
  void InvokeCallback(s16* stream, int byte_len);

  auto GetSampleRate() -> int final;
  auto GetBlockSize() -> int final;
  bool Open(void* userdata, Callback callback) final;
  void SetPause(bool value) final;
  void Close() final;

private:
  Callback callback;
  void* callback_userdata;
  SDL_AudioCallback passthrough = nullptr;
  SDL_AudioDeviceID device;
  SDL_AudioSpec have;
  int want_sample_rate = 48000;
  int want_block_size = 2048;
  bool opened = false;
  bool paused = false;
};

} // namespace nba
