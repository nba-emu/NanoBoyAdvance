/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <emulator/device/audio_device.hpp>

#include "SDL.h"

class SDL2_AudioDevice : public nba::AudioDevice {
public:
  auto GetSampleRate() -> int final { return have.freq; }
  auto GetBlockSize() -> int final { return have.samples; }

  bool Open(void* userdata, Callback callback) final {
    SDL_AudioSpec want;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      LOG_ERROR("SDL_Init(SDL_INIT_AUDIO) failed.");
      return false;
    }

    /* TODO: read from configuration file. */
    want.freq = 48000;
    want.samples = 1024;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.callback = (SDL_AudioCallback)callback;
    want.userdata = userdata;

    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if (device == 0) {
      LOG_ERROR("SDL_OpenAudioDevice: failed to open audio: %s\n", SDL_GetError());
      return false;
    }

    if (have.format != want.format) {
      LOG_ERROR("SDL_AudioDevice: S16 sample format unavailable.");
      return false;
    }

    if (have.channels != want.channels) {
      LOG_ERROR("SDL_AudioDevice: Stereo output unavailable.");
      return false;
    }

    SDL_PauseAudioDevice(device, 0);
    return true;
  }

  void Close() {
    SDL_CloseAudioDevice(device);
  }

private:
  SDL_AudioDeviceID device;
  SDL_AudioSpec have;
};
