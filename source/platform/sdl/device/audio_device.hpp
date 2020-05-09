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

struct SDL2_AudioDevice : public nba::AudioDevice {
  auto GetSampleRate() -> int final { return have.freq; }
  auto GetBlockSize() -> int final { return have.samples; }
  
  auto SetPassthrough(SDL_AudioCallback passthrough) {
    this->passthrough = passthrough;
  }

  void InvokeCallback(std::int16_t* stream, int byte_len) {
    if (callback) {
      callback(callback_userdata, stream, byte_len);
    }
  }

  bool Open(void* userdata, Callback callback) final {
    auto want = SDL_AudioSpec{};

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      LOG_ERROR("SDL_Init(SDL_INIT_AUDIO) failed.");
      return false;
    }

    /* TODO: read from configuration file. */
    want.freq = 48000;
    want.samples = 2048;
    want.format = AUDIO_S16;
    want.channels = 2;
    
    if (passthrough != nullptr) {
      want.callback = passthrough;
      want.userdata = this;
    } else {
      want.callback = (SDL_AudioCallback)callback;
      want.userdata = userdata;
    }

    this->callback = callback;
    callback_userdata = userdata;

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
  Callback callback;
  void* callback_userdata;
  SDL_AudioCallback passthrough = nullptr;
  SDL_AudioDeviceID device;
  SDL_AudioSpec have;
};
