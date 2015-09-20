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

#include <cstdio>
#include <iostream>
#include "core/arm7.h"
#include "core/gba_memory.h"
#include "core/gba_video.h"
#include "common/log.h"
#include "SDL.h"
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

int main(int argc, char **argv)
{
    SDL_Event event;
	GBAMemory* memory;
	ARM7* arm;
    bool running = true;

	if (argc > 1)
	{
		memory = new GBAMemory("bios.bin", argv[1]);
	}
	else
	{
		string rom;
		cout << "Please specify a rom: ";
		cin >> rom;
		if (rom.empty())
		{
			return 0;
		}
		memory = new GBAMemory("bios.bin", rom);
	}

	arm = new ARM7(memory, false);

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

    SDL_WM_SetCaption("NanoBoyAdvance", "NanoBoyAdvance");
    
    while (running)
    {
        ubyte* kb_state = SDL_GetKeyState(NULL);
        ushort joypad = 0;
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
            if (memory->gba_io->ime != 0 && memory->gba_io->if_ != 0)
            {
                arm->FireIRQ();
            }
            arm->Step();
            memory->timer->Step();
            /*memory->gba_io->tm0cnt_l = (memory->gba_io->tm0cnt_l + 1) % 0x10000;
            memory->gba_io->tm1cnt_l = (memory->gba_io->tm1cnt_l + 1) % 0x10000;
            memory->gba_io->tm2cnt_l = (memory->gba_io->tm2cnt_l + 1) % 0x10000;
            memory->gba_io->tm3cnt_l = (memory->gba_io->tm3cnt_l + 1) % 0x10000;*/
            memory->video->Step();
            memory->dma->Step();
            if (memory->video->render_scanline)
            {
                int y = memory->gba_io->vcount;
                for (int x = 0; x < 240; x++)
                {
					setpixel(x, y, memory->video->buffer[y * 240 + x]);
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
