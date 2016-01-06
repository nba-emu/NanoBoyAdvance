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
#include <SDL/SDL.h>
#include <iostream>
#undef main

using namespace std;
using namespace NanoboyAdvance;

SDL_Surface* screen;
uint32_t* buffer;

#define plotpixel(x,y,c) buffer[(y) * 480 + (x)] = c;
void setpixel(int x, int y, int color)
{
    plotpixel(x * 2, y * 2, color);
    plotpixel(x * 2 + 1, y * 2, color);
    plotpixel(x * 2, y * 2 + 1, color);
    plotpixel(x * 2 + 1, y * 2 + 1, color);
}

void step(ARM7* arm, GBAMemory* memory)
{
    if (memory->gba_io->ime != 0 && memory->gba_io->if_ != 0)
    {
        arm->FireIRQ();
    }
    arm->Step();
    memory->timer->Step();
    memory->video->Step();
    memory->dma->Step();
}

// This is our main function / entry point. It gets called when the emulator is started.
int main(int argc, char** argv)
{
    SDL_Event event;
	GBAMemory* memory;
	ARM7* arm;
    CmdLine* cmdline;
    bool running = true;

	if (argc > 1)
	{
        cmdline = parse_parameters(argc, argv);

        // Initialize memory and ARM interpreter core
		memory = new GBAMemory(cmdline->bios_file, cmdline->rom_file);
	    arm = new ARM7(memory, !cmdline->use_bios);
	}
	else
	{
		usage();
        return 0;
	}

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode(480, 320, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        printf("SDL_SetVideoMode Error: %s\n", SDL_GetError());
        SDL_Quit();
    }
    buffer = (uint32_t*)screen->pixels;

    SDL_WM_SetCaption("NanoboyAdvance", "NanoboyAdvance");

    // Main loop
    while (running)
    {
        ubyte* kb_state = SDL_GetKeyState(NULL);
        ushort joypad = 0;

        // Handle keyboard input
        joypad |= kb_state[SDLK_y] ? 0 : 1;
        joypad |= kb_state[SDLK_x] ? 0 : (1 << 1);
        joypad |= kb_state[SDLK_BACKSPACE] ? 0 : (1 << 2);
        joypad |= kb_state[SDLK_RETURN] ? 0 : (1 << 3);
        joypad |= kb_state[SDLK_RIGHT] ? 0 : (1 << 4);
        joypad |= kb_state[SDLK_LEFT] ? 0 : (1 << 5);
        joypad |= kb_state[SDLK_UP] ? 0 : (1 << 6);
        joypad |= kb_state[SDLK_DOWN] ? 0 : (1 << 7);
        joypad |= kb_state[SDLK_s] ? 0 : (1 << 8);
        joypad |= kb_state[SDLK_a] ? 0 : (1 << 9);
        memory->gba_io->keyinput = joypad;

        for (int i = 0; i < 280896; i++)
        {
            step(arm, memory);

            // Copy the finished frame to SDLs pixel buffer
            if (memory->video->render_scanline && memory->gba_io->vcount == 159)
            {
                // Copy data to screen
                for (int y = 0; y < 160; y++)
                {
                    for (int x = 0; x < 240; x++)
                    {
                        setpixel(x, y, memory->video->buffer[y * 240 + x]);
                    }
                }
            }
        }
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
        SDL_Flip(screen);
    }

    SDL_FreeSurface(screen);
    return 0;
}
