/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>
#include <platform/device/sdl_audio_device.hpp>

namespace nba {

static constexpr int k_sample_rate = 48000;

auto SDL3_AudioDevice::GetSampleRate() -> int {
  return k_sample_rate;
}

auto SDL3_AudioDevice::GetBlockSize() -> int {
  // TODO: we do not really have a buffer size anymore. Just remove this from the API?
  return 4096;
}

bool SDL3_AudioDevice::Open(void* userdata, Callback callback) {
  auto want = SDL_AudioSpec{};

  if(SDL_Init(SDL_INIT_AUDIO) < 0) {
    Log<Error>("Audio: SDL_Init(SDL_INIT_AUDIO) failed.");
    return false;
  }

  want.freq = GetSampleRate();
  want.format = SDL_AUDIO_S16;
  want.channels = 2;

  m_callback = callback;
  m_callback_userdata = userdata;

  m_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want, (SDL_AudioStreamCallback)SDL3_AudioDeviceCallback, this);

  if(m_audio_stream == nullptr) {
    Log<Error>("Audio: SDL_OpenAudioDeviceStream: failed to open audio: %s\n", SDL_GetError());
    return false;
  }

  m_opened = true;

  if(!m_paused) {
    SDL_ResumeAudioStreamDevice(m_audio_stream);
  }
  return true;
}

void SDL3_AudioDevice::SetPause(bool value) {
  if(!m_opened) {
    return;
  }

  if(value) {
    SDL_PauseAudioStreamDevice(m_audio_stream);
  } else {
    SDL_ResumeAudioStreamDevice(m_audio_stream);
  }
}

void SDL3_AudioDevice::Close() {
  if(m_opened) {
    SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(m_audio_stream));
    m_opened = false;
  }
}

void SDL3_AudioDeviceCallback(SDL3_AudioDevice* audio_device, SDL_AudioStream* stream, int additional_amount, int total_amount) {
  (void)total_amount;

  std::vector<u8>& tmp_buffer = audio_device->m_tmp_buffer;
  tmp_buffer.resize(additional_amount);
  audio_device->m_callback(audio_device->m_callback_userdata, (s16*)tmp_buffer.data(), additional_amount);
  SDL_PutAudioStreamData(stream, tmp_buffer.data(), additional_amount);
}

} // namespace nba
