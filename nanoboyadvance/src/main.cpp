/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/arm7.h"
#include "core/gba_memory.h"
#include "core/gba_video.h"
#include "common/log.h"
#include "cmdline.h"
#include "debugger.h"
#include <SDL2/SDL.h>
#include <png.h>
#include <iostream>
#undef main

using namespace std;
using namespace NanoboyAdvance;

#define FRAME_CYCLES 280896

// Emulation related globals
GBAMemory* memory;
ARM7* arm;
CmdLine* cmdline;

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

#define plotpixel(x,y,c) buffer[(y) * window_width + (x)] = c;
void setpixel(int x, int y, int color)
{
    int scale_x = window_width / 240;
    int scale_y = window_height / 160;

    // Plot the neccessary amount of pixels
    for (int i = 0; i < scale_x; i++)
    {
        for (int j = 0; j < scale_y; j++)
        {
            plotpixel(x * scale_x + i, y * scale_y + j, color);
        }
    }
}

// Read key input from SDL and feed it into the GBA
void schedule_keyinput()
{
    u8* kb_state = (u8*)SDL_GetKeyboardState(NULL);
    u16 joypad = 0;

    // Check which keys are pressed and set corresponding bits (0 = pressed, 1 = vice versa)
    joypad |= kb_state[SDL_SCANCODE_A] ? 0 : 1;
    joypad |= kb_state[SDL_SCANCODE_S] ? 0 : (1 << 1);
    joypad |= kb_state[SDL_SCANCODE_BACKSPACE] ? 0 : (1 << 2);
    joypad |= kb_state[SDL_SCANCODE_RETURN] ? 0 : (1 << 3);
    joypad |= kb_state[SDL_SCANCODE_RIGHT] ? 0 : (1 << 4);
    joypad |= kb_state[SDL_SCANCODE_LEFT] ? 0 : (1 << 5);
    joypad |= kb_state[SDL_SCANCODE_UP] ? 0 : (1 << 6);
    joypad |= kb_state[SDL_SCANCODE_DOWN] ? 0 : (1 << 7);
    joypad |= kb_state[SDL_SCANCODE_Q] ? 0 : (1 << 8);
    joypad |= kb_state[SDL_SCANCODE_W] ? 0 : (1 << 9);

    // Write the generated value to IO ram
    memory->keyinput = joypad;
}

// Schedules the GBA and generates exactly one frame
inline void schedule_frame()
{
    for (int i = 0; i < FRAME_CYCLES; i++)
    {
        // Interrutps that are enabled *and* pending
        u32 interrupts = memory->interrupt->ie & memory->interrupt->if_;
        
        // Only pause as long as (IE & IF) != 0
        if (memory->halt_state != GBAMemory::GBAHaltState::None && interrupts != 0)
        {
            // If IntrWait only resume if requested interrupt is encountered
            if (!memory->intr_wait || (interrupts & memory->intr_wait_mask) != 0)
            {
                memory->halt_state = GBAMemory::GBAHaltState::None;
                memory->intr_wait = false;
            }
        }

        // Raise an IRQ if neccessary
        if (memory->interrupt->ime && (memory->interrupt->if_ & memory->interrupt->ie))
        {
            #ifdef HARDCORE_DEBUG
            LOG(LOG_INFO, "Run interrupt handler if=0x%x", memory->interrupt->if_);
            #endif
            arm->FireIRQ();
        }

        // Run the hardware's components
        if (memory->halt_state != GBAMemory::GBAHaltState::Stop)
        {
            int forward_steps = 0;

            if (memory->halt_state != GBAMemory::GBAHaltState::Halt)
            {
                arm->cycles = 0;
                arm->Step();
                forward_steps = arm->cycles - 1;
            }

            //for (int j = 0; j <= forward_steps; j++) 
            {
                memory->video->Step();
                memory->Step();

                // Copy the rendered line (if any) to SDLs pixel buffer
                if (memory->video->render_scanline)
                {
                    int y = memory->video->vcount;
                    for (int x = 0; x < 240; x++)
                    {
                        setpixel(x, y, memory->video->buffer[y * 240 + x]);
                    }
                }
            }

            //i += forward_steps;
        }
    }
}

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
    
    // Check for errors during window creation
    if (window == nullptr) 
    {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // Check for errors during renderer creation
    if (renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRenderer Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    // Create SDL surface
    surface = SDL_CreateRGBSurface(0, width, height, 32, 
                                   0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    // Check for errors during window creation
    if (surface == nullptr)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRGBSurface Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    // Create SDL texture
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    // Check for errors...
    if (texture == nullptr) 
    {
        SDL_FreeSurface(surface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        cout << "SDL_CreateTexture Error: " << SDL_GetError()  << endl;
        SDL_Quit();
    }

    // Get pixelbuffer
    buffer = (uint32_t*)surface->pixels;

    // Set width and height globals
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
    {
        png_free(png_ptr, rows[y]);
    }

    png_free(png_ptr, rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

// This is our main function / entry point. It gets called when the emulator is started.
int main(int argc, char** argv)
{
    SDL_Event event;
    bool running = true;
    frameskip_counter = 0;

    if (argc > 1)
    {
        cmdline = parse_parameters(argc, argv);

        // Check for parsing errors
        if (cmdline == NULL)
        {
            usage();
            return 0;
        }

        // Initialize memory and ARM interpreter core
        memory = new GBAMemory(cmdline->bios_file, cmdline->rom_file, "test.sav");
        arm = new ARM7(memory, !cmdline->use_bios);
        
        // Append debugger if desired
        if (cmdline->debug)
        {
            debugger_attach(cmdline, arm, memory);
        }
    }
    else
    {
        usage();
        return 0;
    }

    // Initialize SDL and create window, texture etc..
    sdl_init(240 * cmdline->scale, 160 * cmdline->scale);

    // Run debugger immediatly if specified so
    if (cmdline->debug && cmdline->debug_immediatly)
    {
        debugger_shell();
    }
    
    // Initially setup the frame counter
    frames = 0;
    ticks_start = SDL_GetTicks();

    // Main loop
    while (running)
    {
        int ticks_now;
        u8* kb_state = (u8*)SDL_GetKeyboardState(NULL);

        // Check if cli debugger is requested and run if needed
        if (cmdline->debug && kb_state[SDL_SCANCODE_F11])
        {
            debugger_shell();
        }

        // Dump screenshot on F10
        if (kb_state[SDL_SCANCODE_F10]) create_screenshot();
        
        // Feed keyboard input and generate exactly one frame
        schedule_keyinput();
        schedule_frame();
        frames++;

        // FPS counter
        ticks_now = SDL_GetTicks();
        if (ticks_now - ticks_start >= 1000) 
        {
            string title = "NanoboyAdvance [" + std::to_string(frames) + "fps]";
            SDL_SetWindowTitle(window, title.c_str());

            ticks_start = SDL_GetTicks();
            frames = 0;
        }

        // Update texture
        SDL_UpdateTexture(texture, (const SDL_Rect*)&surface->clip_rect, surface->pixels, surface->pitch);

        // Draw =)
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // We must process SDL events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
    }
    
    // Free stuff (emulator)
    delete arm;
    delete memory;

    // Free stuff (SDL)
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
