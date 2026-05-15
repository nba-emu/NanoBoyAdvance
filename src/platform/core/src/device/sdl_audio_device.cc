// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <platform/device/sdl_audio_device.hh>
#include <atom/logger/logger.hh>

namespace nba {

bool SDL3_AudioDevice::Open(void* userdata, Callback callback) {
  auto specification = SDL_AudioSpec{};

  if(!SDL_Init(SDL_INIT_AUDIO)) {
    ATOM_ERROR("Audio: SDL_Init(SDL_INIT_AUDIO) failed.");
    return false;
  }

  specification.freq = 48000;
  specification.format = SDL_AUDIO_S16LE;
  specification.channels = 2;

  m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &specification, SDL3_AudioDevice::InternalCallback, this);
  if(m_stream == 0) {
    ATOM_ERROR("Audio: SDL_OpenAudioDeviceStream: failed to open audio: %s\n", SDL_GetError());
    return false;
  }

  m_callback = callback;
  m_data.resize(4096); // most systems seem happy with this
  m_specification = specification;
  m_userdata = userdata;

  SDL_ResumeAudioStreamDevice(m_stream);

  return true;
}

void SDL3_AudioDevice::Close() {
  if(m_stream == nullptr) {
    return;
  }

  SDL_DestroyAudioStream(m_stream);
  m_stream = nullptr;
}


void SDL3_AudioDevice::Reset() {
  if(m_stream == nullptr) {
    return;
  }

  SDL_ClearAudioStream(m_stream);
}

void SDL3_AudioDevice::SetPause(bool value) {
  if(m_stream == nullptr) {
    return;
  }

  (value ? SDL_PauseAudioStreamDevice : SDL_ResumeAudioStreamDevice)(m_stream);
}

void SDL3_AudioDevice::InternalCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
  SDL3_AudioDevice* instance = reinterpret_cast<SDL3_AudioDevice*>(userdata);
  if (total_amount > instance->m_data.size()) {
    instance->m_data.resize(total_amount);
  }

  instance->m_callback(instance->m_userdata, instance->m_data.data(), additional_amount);
  SDL_PutAudioStreamDataNoCopy(stream, instance->m_data.data(), additional_amount, nullptr, nullptr);
}

} // namespace nba
