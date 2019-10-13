/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#ifdef _MSC_VER
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#undef main

#include <cstdio>
#include <gba/emulator.hpp>

SDL_Texture*  g_texture;
SDL_Renderer* g_renderer;

class SDL2_AudioDevice : public GameBoyAdvance::AudioDevice {
public:
  auto GetSampleRate() -> int final { return have.freq; }
  auto GetBlockSize() -> int final { return have.samples; }
  
  bool Open(void* userdata, Callback callback) final {
    SDL_AudioSpec want;
    
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      std::puts("SDL2_AudioDevice: SDL_Init(SDL_INIT_AUDIO) failed.");
      return false;
    }
    
    // TODO: read from configuration file.
    want.freq = 48000;
    want.samples = 1024;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.callback = (SDL_AudioCallback)callback;
    want.userdata = userdata;
    
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    
    if (device == 0) {
      std::printf("SDL_OpenAudioDevice: failed to open audio: %s\n", SDL_GetError());
    }
    
    if (have.format != want.format) {
      std::puts("SDL_AudioDevice: S16 sample format unavailable.");
      return false;
    }
    
    if (have.channels != want.channels) {
      std::puts("SDL_AudioDevice: Stereo output unavailable.");
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

class SDL2_InputDevice : public GameBoyAdvance::InputDevice {
public:
  auto Poll(Key key) -> bool final {
    // TODO: Currently we poll events up to 10 times per
    // keypad read. Not sure if this has any effect, but maybe we can fix it.
    
    SDL_PumpEvents();
    
    auto keystate = SDL_GetKeyboardState(NULL);
    
    using Key = InputDevice::Key;
    
    switch (key) {
      case Key::A:
        return keystate[SDL_SCANCODE_Z] || keystate[SDL_SCANCODE_Y];
      case Key::B:
        return keystate[SDL_SCANCODE_X];
      case Key::Select:
        return keystate[SDL_SCANCODE_BACKSPACE];
      case Key::Start:
        return keystate[SDL_SCANCODE_RETURN];
      case Key::Right:
        return keystate[SDL_SCANCODE_RIGHT];
      case Key::Left:
        return keystate[SDL_SCANCODE_LEFT];
      case Key::Up:
        return keystate[SDL_SCANCODE_UP];
      case Key::Down:
        return keystate[SDL_SCANCODE_DOWN];
      case Key::R:
        return keystate[SDL_SCANCODE_S];
      case Key::L:
        return keystate[SDL_SCANCODE_A];
    }
    
    return false;
  }
};

class SDL2_VideoDevice : public GameBoyAdvance::VideoDevice {
public:
  
  void Draw(std::uint32_t* buffer) final {
    SDL_UpdateTexture(g_texture, nullptr, buffer, 240 * sizeof(std::uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
  }
};

int main(int argc, char** argv) {
  bool running = true;
  bool fullscreen = false;
  SDL_Event event;
  SDL_Window* window;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("NanoboyAdvance",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                480,
                320,
                0
               );
  g_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
  
  if (argc != 2) {
    std::printf("Usage: %s rom_path\n", argv[0]);
    return -1;
  }
  
  std::string rom_path = argv[1];
  
  auto config = std::make_shared<GameBoyAdvance::Config>();
  
  config->audio_dev = std::make_shared<SDL2_AudioDevice>();
  config->input_dev = std::make_shared<SDL2_InputDevice>();
  config->video_dev = std::make_shared<SDL2_VideoDevice>();
  
  auto emulator = std::make_unique<GameBoyAdvance::Emulator>(config);
  
  if (!emulator->LoadGame(rom_path)) {
    std::printf("Cannot open ROM: %s\n", rom_path.c_str());
    return -1;
  }

  /* Benchmark */
  int frames = 0;
  int ticks1 = SDL_GetTicks();

  while (running) {
    emulator->Frame();
    frames++;
    
    int ticks2 = SDL_GetTicks();
    if ((ticks2 - ticks1) >= 1000) {
      std::printf("FPS: %d (%.2f%%)\n", frames, frames/60.0*100.0);
      ticks1 = ticks2;
      frames = 0;
    }

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        auto key_event = (SDL_KeyboardEvent*)(&event);
        
        switch (key_event->keysym.sym) {
          case SDLK_SPACE:
            if (event.type == SDL_KEYDOWN) {
              SDL_GL_SetSwapInterval(0);
            } else {
              SDL_GL_SetSwapInterval(1);
            }
            break;
          case SDLK_F9:
            if (event.type == SDL_KEYUP) {
              emulator->Reset();
            }
            break;
          case SDLK_F10:
            if (event.type == SDL_KEYUP) {
              fullscreen = !fullscreen;
              SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
            }
            break;
        }
      }
    }
  }

  SDL_DestroyTexture(g_texture);
  SDL_DestroyRenderer(g_renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
