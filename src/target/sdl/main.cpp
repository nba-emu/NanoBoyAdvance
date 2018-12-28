/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef _MSC_VER
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#undef main

#include <cstdio>
#include "core/cpu.hpp"

NanoboyAdvance::GBA::CPU cpu;

int main(int argc, char** argv) {
    bool running = true;
    SDL_Event event;
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("NanoboyAdvance 0.1",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              480,
                              320,
                              0
                             );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);

    size_t size;
    auto file = std::fopen("armwrestler.gba", "rb");
    std::uint8_t* rom;

    if (file == nullptr) {
        std::puts("Error: unable to open armwrestler.gba");
        return -1;
    }

    std::fseek(file, 0, SEEK_END);
    size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    rom = new uint8_t[size];
    if (std::fread(rom, 1, size, file) != size) {
        std::puts("Error: unable to fully read the ROM.");
        return -1;
    }

    /* Initialize emulator. */
    cpu.Reset();
    cpu.SetSlot1(rom, size);

    while (running) {
        /* generate frame here. */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    delete rom;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
