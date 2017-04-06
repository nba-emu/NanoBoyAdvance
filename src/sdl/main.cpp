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
#include "gba/cpu/cpu.hpp"
#include "util/file.hpp"

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

void sound_cb(APU* apu, u16* stream, int length) {
    apu->fill_buffer(stream, length);
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
    
    CPU emu(config);

    u16* keyinput;
    u32  fbuffer[240 * 160];
    
    int scale = 1;
    std::string rom_path;
    std::string save_path;
    
    ArgumentParser parser;
    shared_ptr<ParsedArguments> args;
    
    Option bios_opt("bios", "path", false);
    Option save_opt("save", "path", true);
    Option scale_opt("scale", "factor", true);
    Option speed_opt("forward", "multiplier", true);
    Option frameskip_opt("frameskip", "frames", true);
    Option darken_opt("darken", "", true);
    
    parser.add_option(bios_opt);
    parser.add_option(save_opt);
    parser.add_option(scale_opt);
    parser.add_option(speed_opt);
    parser.add_option(frameskip_opt);
    parser.add_option(darken_opt);
    
    parser.set_file_limit(1, 1);
    
    args = parser.parse(argc, argv);
    
    if (args->error) {
        return -1;
    }
    
    try {
        rom_path = args->files[0];
        
        args->get_option_string("bios", config->bios_path);
        
        if (args->option_exists("save")) {
            args->get_option_string("save", save_path);
        } else {
            save_path = rom_path.substr(0, rom_path.find_last_of(".")) + ".sav";
        }
        
        if (args->option_exists("scale")) {
            args->get_option_int("scale", scale);
            
            if (scale == 0) {
                parser.print_usage(argv[0]);
                return -1;
            }
        }
        
        if (args->option_exists("forward")) {
            args->get_option_int("forward", config->multiplier);
            
            if (config->multiplier == 0) {
                parser.print_usage(argv[0]);
                return -1;
            }
        }
        
        if (args->option_exists("frameskip")) {
            args->get_option_int("frameskip", config->frameskip);
        }
        
        if (args->option_exists("darken")) {
            args->get_option_bool("darken", config->darken_screen);
        }
    } catch (std::exception& e) {
        parser.print_usage(argv[0]);
        return -1;
    }
    
    config->framebuffer = fbuffer;
    
    emu.load_config();
    
    if (File::exists(rom_path) && File::exists(config->bios_path)) {
        emu.load_game(rom_path, save_path);
        keyinput = &emu.get_keypad();
    } else {
        parser.print_usage(argv[0]);
        return -1;
    }
    
    // setup SDL window
    g_width  *= scale;
    g_height *= scale;
    setup_window();
    
    // setup SDL sound
    setup_sound(&emu.get_apu());
    
    SDL_Event event;
    bool running = true;
    
    int frames = 0;
    int ticks1 = SDL_GetTicks();
    
    while (running) {
        
        // generate frame(s)
        emu.frame();
        
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
                        //emu.load_config();
                        
                        continue;
                    case SDLK_F9:
                        emu.reset();
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