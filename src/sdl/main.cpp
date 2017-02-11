///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


//#include "gba/gba.h"
#include "gba/cpu.hpp"
#include "util/file.hpp"
#include "util/bit.hpp"
#include "arguments.h"
#include <SDL2/SDL.h>
#include <png.h>
#include <iostream>
#include <string>
#undef main

using namespace std;
using namespace util;

gba::CPU* emu;
Arguments* args;

// SDL related globals
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Surface* surface;
SDL_Texture* texture;
uint32_t* buffer;
int window_width;
int window_height;
int frameskip_counter;

// FPS counter
int ticks_start;
int frames;

// Initializes SDL2 and stuff
void sdl_init(int width, int height)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        SDL_Quit();
    }

    // Create SDL2 Window
    window = SDL_CreateWindow("NanoboyAdvance", 100, 100, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRenderer Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    // Create SDL surface
    surface = SDL_CreateRGBSurface(0, width, height, 32,
                                   0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    if (surface == nullptr)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRGBSurface Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    // Create SDL texture
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr)
    {
        SDL_FreeSurface(surface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        cout << "SDL_CreateTexture Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    buffer = (uint32_t*)surface->pixels;
    window_width = width;
    window_height = height;
}

void create_screenshot()
{
    FILE* fp = fopen("screenshot.png", "wb");
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte** rows = NULL;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    png_set_IHDR(png_ptr,
                 info_ptr,
                 window_width,
                 window_height,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    rows = (png_byte**)png_malloc(png_ptr, window_height * sizeof(png_byte*));

    for (int y = 0; y < window_height; y++)
    {
        png_byte* row = (png_byte*)png_malloc(png_ptr, window_width * sizeof(u32));
        rows[y] = row;

        for (int x = 0; x < window_width; x++)
        {
            u32 argb = buffer[y * window_width + x];
            *row++ = (argb >> 16) & 0xFF;
            *row++ = (argb >> 8) & 0xFF;
            *row++ = argb & 0xFF;
            *row++ = (argb >> 24) & 0xFF;
        }
    }

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    for (int y = 0; y < window_height; y++)
        png_free(png_ptr, rows[y]);

    png_free(png_ptr, rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main(int argc, char** argv)
{
    SDL_Event event;
    bool running = true;
    u32* video_buffer;
    u16* keyinput;

    if (argc > 1)
    {
        string rom_file;
        string save_file;

        args = parse_args(argc, argv);
        if (args == nullptr)
        {
            usage();
            return 0;
        }

        rom_file = string(args->rom_file);
        save_file = rom_file.substr(0, rom_file.find_last_of(".")) + ".sav";

        if (file::exists(rom_file))
        {
            u8* rom = file::read_data(rom_file);
            size_t rom_size = file::get_size(rom_file);
            u8* bios = file::read_data(string(args->bios_file));
            size_t bios_size = file::get_size(string(args->bios_file));

            emu = new gba::CPU();

            emu->set_bios(bios, bios_size);
            emu->set_game(rom, rom_size, save_file);
            emu->reset();

            keyinput = &emu->get_keypad();

            gba::ppu& ppu = emu->get_ppu();

            ppu.set_frameskip(args->speedup);
            video_buffer = ppu.get_framebuffer();
        }
    }
    else
    {
        usage();
        return 0;
    }

    // Initialize SDL and create window, texture etc..
    sdl_init(240 * args->scale, 160 * args->scale);

    // Initialize framecounter
    frames = 0;
    ticks_start = SDL_GetTicks();

    int scale = args->scale;

    // Main loop
    while (running)
    {
        int ticks_now;
        u8* kb_state = (u8*)SDL_GetKeyboardState(NULL);

        // Reset game on F9
        if (kb_state[SDL_SCANCODE_F9])
            emu->reset();

        // Dump screenshot on F10
        if (kb_state[SDL_SCANCODE_F10])
            create_screenshot();

        bit::set<u16,0>(*keyinput, !kb_state[SDL_SCANCODE_A]);
        bit::set<u16,1>(*keyinput, !kb_state[SDL_SCANCODE_S]);
        bit::set<u16,2>(*keyinput, !kb_state[SDL_SCANCODE_BACKSPACE]);
        bit::set<u16,3>(*keyinput, !kb_state[SDL_SCANCODE_RETURN]);
        bit::set<u16,4>(*keyinput, !kb_state[SDL_SCANCODE_RIGHT]);
        bit::set<u16,5>(*keyinput, !kb_state[SDL_SCANCODE_LEFT]);
        bit::set<u16,6>(*keyinput, !kb_state[SDL_SCANCODE_UP]);
        bit::set<u16,7>(*keyinput, !kb_state[SDL_SCANCODE_DOWN]);
        bit::set<u16,8>(*keyinput, !kb_state[SDL_SCANCODE_W]);
        bit::set<u16,9>(*keyinput, !kb_state[SDL_SCANCODE_Q]);

        for (int i = 0; i < args->speedup; i++)
        {
            emu->frame();
            frames++;
        }

        for (int y = 0; y < window_height; y++)
        {
            for (int x = 0; x < window_width; x++)
            {
                int src_x = x / scale;
                int src_y = y / scale;
                buffer[(y * window_width) + x] = video_buffer[(src_y * 240) + src_x];
            }
        }

        // Update FPS counter
        ticks_now = SDL_GetTicks();
        if (ticks_now - ticks_start >= 1000)
        {
            int percentage = (frames / 60.0) * 100;
            int rendered_frames = frames / args->speedup;

            string title = "NanoboyAdvance [" + std::to_string(percentage) + "% | " +
                                                std::to_string(rendered_frames) + "fps]";
            SDL_SetWindowTitle(window, title.c_str());

            ticks_start = SDL_GetTicks();
            frames = 0;
        }

        // Update texture and draw
        SDL_UpdateTexture(texture, (const SDL_Rect*)&surface->clip_rect, surface->pixels, surface->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Process SDL events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }
    }

    // Free stuff (emulator)
    delete emu;

    // Free stuff (SDL)
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
