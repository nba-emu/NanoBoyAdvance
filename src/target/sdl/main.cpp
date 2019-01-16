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
#include "core/config.hpp"

NanoboyAdvance::GBA::Config config;
NanoboyAdvance::GBA::CPU cpu(&config);

int main(int argc, char** argv) {
    bool running = true;
    SDL_Event event;
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;

    std::uint32_t fb[240*160];

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
    auto file = std::fopen("pmdtr.gba", "rb");
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

    /* Setup configuration object. */
    config.video.output = fb;

    /* Initialize emulator. */
    cpu.SetSlot1(rom, size);

    /* Benchmark */
    int frames = 0;
    int ticks1 = SDL_GetTicks();

    while (running) {
        cpu.RunFor(280896);

        frames++;
        int ticks2 = SDL_GetTicks();
        if ((ticks2 - ticks1) >= 1000) {
            std::printf("FPS: %d\n", frames);
            ticks1 = ticks2;
            frames = 0;
        }

        /* generate frame here. */
        SDL_UpdateTexture(texture, nullptr, fb, 240 * sizeof(std::uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                std::uint16_t bit;

                auto key_event = (SDL_KeyboardEvent*)(&event);

                switch (key_event->keysym.sym) {
                    case SDLK_z:
                    case SDLK_y: bit = (1<<0); break;
                    case SDLK_x: bit = (1<<1); break;
                    case SDLK_BACKSPACE: bit = (1<<2); break;
                    case SDLK_RETURN: bit = (1<<3); break;
                    case SDLK_RIGHT:  bit = (1<<4); break;
                    case SDLK_LEFT:   bit = (1<<5); break;
                    case SDLK_UP:     bit = (1<<6); break;
                    case SDLK_DOWN:   bit = (1<<7); break;
                    case SDLK_w: bit = (1<<8); break;
                    case SDLK_q: bit = (1<<9); break;
                }
                
                if (event.type == SDL_KEYUP) {
                    cpu.mmio.keyinput |=  bit;
                } else {
                    cpu.mmio.keyinput &= ~bit;
                }
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
