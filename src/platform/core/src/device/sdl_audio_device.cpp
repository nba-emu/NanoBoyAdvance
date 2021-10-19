/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>
#include <platform/device/sdl_audio_device.hpp>

namespace nba {

void SDL2_AudioDevice::SetSampleRate(int sample_rate) {
  want_sample_rate = sample_rate;
}

void SDL2_AudioDevice::SetBlockSize(int block_size) {
  want_block_size = block_size;
}

void SDL2_AudioDevice::SetPassthrough(SDL_AudioCallback passthrough) {
  this->passthrough = passthrough;
}

void SDL2_AudioDevice::InvokeCallback(s16* stream, int byte_len) {
  if (callback) {
    callback(callback_userdata, stream, byte_len);
  }
}

auto SDL2_AudioDevice::GetSampleRate() -> int {
  return have.freq;
}

auto SDL2_AudioDevice::GetBlockSize() -> int {
  return have.samples;
}

bool SDL2_AudioDevice::Open(void* userdata, Callback callback) {
  auto want = SDL_AudioSpec{};

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    Log<Error>("Audio: SDL_Init(SDL_INIT_AUDIO) failed.");
    return false;
  }

  want.freq = want_sample_rate;
  want.samples = want_block_size;
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
    Log<Error>("Audio: SDL_OpenAudioDevice: failed to open audio: %s\n", SDL_GetError());
    return false;
  }

  opened = true;

  if (have.format != want.format) {
    Log<Error>("Audio: SDL_AudioDevice: S16 sample format unavailable.");
    return false;
  }

  if (have.channels != want.channels) {
    Log<Error>("Audio: SDL_AudioDevice: Stereo output unavailable.");
    return false;
  }

  if (!paused) {
    SDL_PauseAudioDevice(device, 0);
  }
  return true;
}

void SDL2_AudioDevice::SetPause(bool value) {
  if (opened) {
    SDL_PauseAudioDevice(device, value ? 1 : 0);
  }
}

void SDL2_AudioDevice::Close() {
  if (opened) {
    SDL_CloseAudioDevice(device);
    opened = false;
  }
}

} // namespace nba
