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
#include <SDL2/SDL.h>
#include "gba/cpu.hpp"
#include "util/file.hpp"

#include "argument_parser.hpp"

#undef main

using namespace std;
using namespace Util;
using namespace GameBoyAdvance;

struct GBAConfig {
    std::string rom_path;
    std::string save_path;
    std::string bios_path;
    
    int scale = 1;
    int speed = 1;
};

int g_width  = 240;
int g_height = 160;

SDL_Window*   g_window;
SDL_Texture*  g_texture;
SDL_Renderer* g_renderer;

void setup_window() {
    SDL_Init(SDL_INIT_EVERYTHING);

    g_window = SDL_CreateWindow("NanoboyAdvance", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED,
                                  g_width,
                                  g_height,
                                  0
                               );
    
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    g_texture  = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
}

void free_window() {
    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
}

int main(int argc, char** argv) {
    CPU emu;
    u32* fbuffer;
    
    ArgumentParser parser;
    shared_ptr<ParsedArguments> args;
    shared_ptr<GBAConfig> config(new GBAConfig());
    
    Option bios_opt("bios", "path", false);
    Option save_opt("save", "path", true);
    Option scale_opt("scale", "factor", true);
    Option speed_opt("speed", "multiplier", true);
    
    parser.add_option(bios_opt);
    parser.add_option(save_opt);
    parser.add_option(scale_opt);
    parser.add_option(speed_opt);
    
    parser.set_file_limit(1, 1);
    
    args = parser.parse(argc, argv);
    
    if (args->error) {
        return -1;
    }
    
    try {
        std::string rom = args->files[0];
        
        config->rom_path = rom;
        
        args->get_option_string("bios", config->bios_path);
        
        if (args->option_exists("save")) {
            args->get_option_string("save", config->save_path);
        } else {
            config->save_path = rom.substr(0, rom.find_last_of(".")) + ".sav";
        }
        
        if (args->option_exists("scale")) {
            args->get_option_int("scale", config->scale);
            
            if (config->scale == 0) {
                parser.print_usage(argv[0]);
                return -1;
            }
        }
        
        if (args->option_exists("speed")) {
            args->get_option_int("speed", config->speed);
            
            if (config->speed == 0) {
                parser.print_usage(argv[0]);
                return -1;
            }
        }
    } catch (std::exception& e) {
        parser.print_usage(argv[0]);
        return -1;
    }
    
    if (File::exists(config->rom_path) && File::exists(config->bios_path)) {
        
        u8* rom_data = File::read_data(config->rom_path);
        int rom_size = File::get_size (config->rom_path);
        
        u8* bios_data = File::read_data(config->bios_path);
        int bios_size = File::get_size (config->bios_path);
        
        emu.set_game(rom_data,  rom_size, config->save_path);
        emu.set_bios(bios_data, bios_size);
        emu.reset();
        
        PPU& ppu = emu.get_ppu();
        
        ppu.set_frameskip(config->speed);
        
        fbuffer = ppu.get_framebuffer();
    } else {
        parser.print_usage(argv[0]);
        return -1;
    }
    
    g_width  *= config->scale;
    g_height *= config->scale;
    setup_window();
    
    SDL_Event event;
    bool running = true;
    
    while (running) {
        for (int frame = 0; frame < config->speed; frame++) {
            emu.frame();
        }
        
        SDL_UpdateTexture(g_texture, NULL, fbuffer, g_width * sizeof(u32));
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
        SDL_RenderPresent(g_renderer);
    }
    
    free_window();
    SDL_Quit();
    
    return 0;
}