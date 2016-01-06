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
#include <iomanip>
#include <vector>
#include "core/arm7.h"
#include "core/gba_memory.h"
#include "core/gba_video.h"
#include "common/log.h"
#include <SDL/SDL.h>
#undef main

using namespace std;
using namespace NanoboyAdvance;

SDL_Surface* screen;
uint32_t* buffer;
bool step_frame;

#define display_word setfill('0') << setw(8) << hex
#define display_hword setfill('0') << setw(4) << hex
#define display_byte setfill('0') << setw(2) << hex

#define plotpixel(x,y,c) buffer[(y) * 480 + (x)] = c;
void setpixel(int x, int y, int color)
{
    plotpixel(x * 2, y * 2, color);
    plotpixel(x * 2 + 1, y * 2, color);
    plotpixel(x * 2, y * 2 + 1, color);
    plotpixel(x * 2 + 1, y * 2 + 1, color);
}

/*void arm_print_registers(ARM7* arm)
{
    // First line
    cout << "r0=0x" << display_word << arm->GetGeneralRegister(0) << "\t";
    cout << "r4=0x" << display_word << arm->GetGeneralRegister(4) << "\t";
    cout << "r8=0x" << display_word << arm->GetGeneralRegister(8) << "\t";
    cout << "r12=0x" << display_word << arm->GetGeneralRegister(12) << endl;

    // Second line
    cout << "r1=0x" << display_word << arm->GetGeneralRegister(1) << "\t";
    cout << "r5=0x" << display_word << arm->GetGeneralRegister(5) << "\t";
    cout << "r9=0x" << display_word << arm->GetGeneralRegister(9) << "\t";
    cout << "r13=0x" << display_word << arm->GetGeneralRegister(13) << endl;

    // Third line
    cout << "r2=0x" << display_word << arm->GetGeneralRegister(2) << "\t";
    cout << "r6=0x" << display_word << arm->GetGeneralRegister(6) << "\t";
    cout << "r10=0x" << display_word << arm->GetGeneralRegister(10) << "\t";
    cout << "r14=0x" << display_word << arm->GetGeneralRegister(14) << endl;

    // Fourth line
    cout << "r3=0x" << display_word << arm->GetGeneralRegister(3) << "\t";
    cout << "r7=0x" << display_word << arm->GetGeneralRegister(7) << "\t";
    cout << "r11=0x" << display_word << arm->GetGeneralRegister(11) << "\t";
    cout << "r15=0x" << display_word << arm->GetGeneralRegister(15) << ((arm->GetStatusRegister() & 0x10) ? " (Thumb)" : " (ARM)") << endl;

    // Fifth line
    cout << "cpsr=0x" << display_word << arm->GetStatusRegister() << "\t";
    cout << "spsr_<mode>=0x" << display_word << arm->GetSavedStatusRegister() << endl;
}*/

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

/*ubyte take_byte(string parameter)
{
    ubyte result = static_cast<ubyte>(stoul(parameter, nullptr, 0));
    if (result > 0xFF)
    {
        throw new out_of_range("Parameters exceeds max. limit of byte.");
    }
    return result;
}

ushort take_hword(string parameter)
{
    ushort result = static_cast<ushort>(stoul(parameter, nullptr, 0));
    if (result > 0xFFFF)
    {
        throw new out_of_range("Parameters exceeds max. limit of byte.");
    }
    return result;
}

uint take_word(string parameter)
{
    return stoul(parameter, nullptr, 0);
}*/

void debugger(ARM7* arm, GBAMemory* memory, bool complain)
{
    puts("About to be reimplemented :/");
}

// Called when none or invalid arguments are passed
void usage()
{
    cout << "Usage: ./nanoboyadvance [--debug] [--complain] [--bios bios_file] rom_file" << endl;
}

// This is our main function / entry point. It gets called when the emulator is started.
int main(int argc, char **argv)
{
    SDL_Event event;
	GBAMemory* memory;
	ARM7* arm;
    bool running = true;
    bool emulate_bios = true;
    bool run_debugger = false;
    bool complain = false; // determines wether the emulator will enter debug state on bad memory accesses.
    string bios_file = "bios.bin";
    string rom_file = "";

    step_frame = false;

	if (argc > 1)
	{
        int current_argument = 1;

        // Process arguments
        while (current_argument < argc)
        {
            bool parameter_no_option = false;

            // Handle optional parameters
            if (strcmp(argv[current_argument], "--bios") == 0)
            {
                emulate_bios = false;
                if (argc > current_argument + 1)
                {
                    bios_file = argv[++current_argument];
                }
                else
                {
                    usage();
                    return 0;
                }
            }
            else if (strcmp(argv[current_argument], "--debug") == 0)
            {
                run_debugger = true;
            }
            else if (strcmp(argv[current_argument], "--complain") == 0)
            {
                complain = true;
            }
            else
            {
                parameter_no_option = true;
            }

            // Get rom path
            if (current_argument == argc - 1)
            {
                if (!parameter_no_option)
                {
                    usage();
                    return 0;
                }
                rom_file = argv[current_argument];
            }

            current_argument++;
        }

        // Initialize memory and ARM interpreter core
		memory = new GBAMemory(bios_file, rom_file);
	    arm = new ARM7(memory, emulate_bios);
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

    // Run debugger if stated so
    if (run_debugger)
    {
        debugger(arm, memory, complain);
    }

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

        // Schedule VBA-SDL-H like console interface debugger
        if (kb_state[SDLK_F11])
        {
            debugger(arm, memory, complain);
        }

        for (int i = 0; i < 280896; i++)
        {
            step(arm, memory);

            // Copy the finished frame to SDLs pixel buffer
            if (memory->video->render_scanline && memory->gba_io->vcount == 159)
            {
                if (step_frame)
                {
                    step_frame = false; // do only break for the current frame
                    debugger(arm, memory, complain);
                }

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
