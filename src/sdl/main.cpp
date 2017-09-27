/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
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

#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include "core/emulator/emulator.hpp"
#include "util/file.hpp"
#include "util/ini.hpp"

#include "argument_parser.hpp"

#undef main

using namespace std;
using namespace Util;
using namespace GameBoyAdvance;

int g_width  = 240;
int g_height = 160;

SDL_Window*   g_window;
SDL_Texture*  g_texture;
SDL_Renderer* g_renderer;

bool g_fullscreen = false;

void sound_cb(APU* apu, u16* stream, int length) {
    apu->fillBuffer(stream, length);
}

void setup_sound(APU* apu) {
    SDL_AudioSpec spec;

    spec.freq     = 44100;
    spec.samples  = 1024;
    spec.format   = AUDIO_U16;
    spec.channels = 2;
    spec.callback = sound_cb;
    spec.userdata = apu;

    SDL_Init(SDL_INIT_AUDIO);
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void setup_window() {
    SDL_Init(SDL_INIT_VIDEO);

    g_window = SDL_CreateWindow("NanoboyAdvance",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  g_width,
                                  g_height,
                                  0
                               );

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    g_texture  = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
}

void free_window() {
    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
}

int main(int argc, char** argv) {

    Config* config = new Config();

    Emulator emu(config);

    u16* keyinput;
    u32  fbuffer[240 * 160];

    int scale = 1;
    std::string rom_path = std::string(argv[1]);

    INI ini("config.ini");

    // TODO implement defaults
    // [Emulation]
    config->bios_path  = ini.getString("Emulation", "biosPath");
    config->multiplier = ini.getInteger("Emulation", "fastForward");

    //if (config->multiplier > 1) {

    //}

    // [Video]
    scale                 = ini.getInteger("Video", "scale");
    config->darken_screen = ini.getInteger("Video", "darken"); // boolean!
    config->frameskip     = ini.getInteger("Video", "frameskip");

    if (scale < 1) scale = 1;

    config->framebuffer = fbuffer;

    emu.reloadConfig();

    if (File::exists(rom_path) && File::exists(config->bios_path)) {
        auto cart = Cartridge::fromFile(rom_path);

        emu.loadGame(cart);
        keyinput = &emu.getKeypad();
    } else {
        //parser.print_usage(argv[0]);
        return -1;
    }

    // setup SDL window
    g_width  *= scale;
    g_height *= scale;
    setup_window();

    // setup SDL sound
    setup_sound(&emu.getAPU());

    SDL_Event event;
    bool running = true;

    int frames = 0;
    int ticks1 = SDL_GetTicks();

    while (running) {

        // generate frame(s)
        emu.runFrame();

        // update frame counter
        frames += config->fast_forward ? config->multiplier : 1;

        int ticks2 = SDL_GetTicks();
        if (ticks2 - ticks1 >= 1000) {
            int percentage = (frames / 60.0) * 100;
            int rendered_frames = frames;

            if (config->fast_forward) {
                rendered_frames /= config->multiplier;
            }

            string title = "NanoboyAdvance [" + std::to_string(percentage) + "% | " +
                                                std::to_string(rendered_frames) + "fps]";
            SDL_SetWindowTitle(g_window, title.c_str());

            ticks1 = ticks2;
            frames = 0;
        }

        // send generated frame to texture
        SDL_UpdateTexture(g_texture, NULL, fbuffer, 240 * sizeof(u32));

        // tell SDL to draw the texture
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
        SDL_RenderPresent(g_renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                int  num;
                bool released = event.type == SDL_KEYUP;

                SDL_KeyboardEvent* key_event = (SDL_KeyboardEvent*)(&event);

                switch (key_event->keysym.sym) {
                    case SDLK_z:
                    case SDLK_y:         num = (1<<0); break;
                    case SDLK_x:         num = (1<<1); break;
                    case SDLK_BACKSPACE: num = (1<<2); break;
                    case SDLK_RETURN:    num = (1<<3); break;
                    case SDLK_RIGHT:     num = (1<<4); break;
                    case SDLK_LEFT:      num = (1<<5); break;
                    case SDLK_UP:        num = (1<<6); break;
                    case SDLK_DOWN:      num = (1<<7); break;
                    case SDLK_w:         num = (1<<8); break;
                    case SDLK_q:         num = (1<<9); break;
                    case SDLK_SPACE:
                        if (released) {
                            config->fast_forward = false;
                        } else {
                            // prevent more than one update!
                            if (config->fast_forward) {
                                continue;
                            }
                            config->fast_forward = true;
                        }

                        //// make core adapt new settings
                        //emu.reloadConfig();

                        continue;
                    case SDLK_F9:
                        emu.reset();
                        continue;
                    case SDLK_F10:
                        if (!released) continue;

                        if (g_fullscreen) {
                            SDL_SetWindowFullscreen(g_window, 0);
                        } else {
                            SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        g_fullscreen = !g_fullscreen;
                        continue;
                    default:
                        continue;
                }

                if (released) {
                    *keyinput |= num;
                } else {
                    *keyinput &= ~num;
                }
            }
        }
    }

    free_window();
    SDL_Quit();

    return 0;
}
