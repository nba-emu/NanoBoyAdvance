/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <emulator/device/audio_device.hpp>

#include "SDL.h"

class SDL2_VideoDevice : public nba::VideoDevice {
public:
  SDL2_VideoDevice(SDL_Window* window) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
    texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
    SDL_RenderSetLogicalSize(renderer, 240, 160);
  }

 ~SDL2_VideoDevice() final {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
  }

  void Draw(std::uint32_t* buffer) final {
    SDL_UpdateTexture(texture, nullptr, buffer, 240 * sizeof(std::uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }

private:
  SDL_Texture* texture;
  SDL_Renderer* renderer;
};
