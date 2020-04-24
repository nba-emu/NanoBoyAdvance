/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstdio>
#include <common/framelimiter.hpp>
#include <common/log.hpp>
#include <emulator/config/config_toml.hpp>
#include <emulator/emulator.hpp>
#include <fmt/format.h>

#include "device/audio_device.hpp"
#include "device/input_device.hpp"
#include "device/video_device.hpp"

#undef main

int main(int argc, char** argv) {
  bool running = true;
  bool fullscreen = false;

  SDL_Event event;
  SDL_Window* window;
  common::Framelimiter framelimiter;

  common::logger::init();

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  window = SDL_CreateWindow("NanoboyAdvance",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                480,
                320,
                SDL_WINDOW_RESIZABLE
               );
  
  if (argc != 2) {
    fmt::print("Usage: {0} rom_path\n", argv[0]);
    return -1;
  }
  
  std::string rom_path = argv[1];
  
  auto config = std::make_shared<nba::Config>();
  
  config->audio_dev = std::make_shared<SDL2_AudioDevice>();
  config->input_dev = std::make_shared<SDL2_InputDevice>();
  config->video_dev = std::make_shared<SDL2_VideoDevice>(window);
  config_toml_read(*config, "config.toml");
  
  auto emulator = std::make_unique<nba::Emulator>(config);
  auto status = emulator->LoadGame(rom_path);
  
  using StatusCode = nba::Emulator::StatusCode;
  
  if (status != StatusCode::Ok) {
    switch (status) {
      case StatusCode::GameNotFound: {
        fmt::print("Cannot open ROM: {0}\n", rom_path);
        return -1;
      }
      case StatusCode::BiosNotFound: {
        fmt::print("Cannot open BIOS: {0}\n", config->bios_path);
        return -2;
      }
      case StatusCode::GameWrongSize: {
        fmt::print("The provided ROM file is larger than the allowed 32 MiB.\n");
        return -3;
      }
      case StatusCode::BiosWrongSize: {
        fmt::print("The provided BIOS file does not match the expected size of 16 KiB.\n");
        return -4;
      }
    }
    
    return -1;
  }

  framelimiter.Reset(16777216.0 / 280896.0); // ~ 59.7 fps
  SDL_GL_SetSwapInterval(0);
  
  while (running) {
    framelimiter.Run([&] {
      emulator->Frame();

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          running = false;
        } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
          auto key_event = (SDL_KeyboardEvent*)(&event);

          switch (key_event->keysym.sym) {
            case SDLK_SPACE: {
              framelimiter.Unbounded(event.type == SDL_KEYDOWN);
              break;
            }
            case SDLK_F9: {
              if (event.type == SDL_KEYUP) {
                emulator->Reset();
              }
              break;
            }
            case SDLK_F10: {
              if (event.type == SDL_KEYUP) {
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
              }
              break;
            }
          }
        }
      }

      SDL_PumpEvents();
    }, [&](int fps) {
    });
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
