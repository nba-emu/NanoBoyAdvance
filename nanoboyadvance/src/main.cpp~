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

void arm_print_registers(ARM7* arm)
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

ubyte take_byte(string parameter)
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
}

void debugger(ARM7* arm, GBAMemory* memory)
{
    bool debugging = true;

    // Welcome message
    cout << "Nanoboy Shell (nsh)" << endl << endl;

    // TODO: Handle step over breakpoint
    if (arm->hit_breakpoint)
    {
        ARM7Breakpoint* breakpoint = arm->last_breakpoint;

        // Breakpoint type specific code
        switch (breakpoint->breakpoint_type)
        {
        case ARM7Breakpoint::ARM7BreakpointType::Execute:
            cout << "Hit hardware breakpoint 0x" << display_word << breakpoint->concerned_address << endl << endl;
            break;
        case ARM7Breakpoint::ARM7BreakpointType::Read:
            cout << "Read from 0x" << display_word << breakpoint->concerned_address << endl << endl;
            break;
        case ARM7Breakpoint::ARM7BreakpointType::Write:
            cout << "Write to 0x" << display_word << breakpoint->concerned_address << endl << endl;
            break;
        case ARM7Breakpoint::ARM7BreakpointType::Access:
            // TODO
            break;
        case ARM7Breakpoint::ARM7BreakpointType::StepOver:
            debugging = false;
            // TODO...
            return;
        case ARM7Breakpoint::ARM7BreakpointType::SVC:
            cout << "Software Interrupt 0x" << display_byte << breakpoint->bios_call << endl << endl;
            break;
        }
    }
    else if (arm->crashed)
    {
        switch (arm->crash_reason)
        {
        case ARM7::CrashReason::BadPC:
            cout << "Program Counter in unusual / bad memory area (0x" << display_word << arm->GetGeneralRegister(15) << ")" << endl;
            break;
        }
    }

    // Display registers
    arm_print_registers(arm);

    // Process commands until user continues execution
    while (debugging)
    {
        string command;
        vector<string> tokens;

        // Get next command
        cout << "nanoboy ~# ";
        getline(cin, command);

        // Don't process empty input
        if (command.length() == 0)
        {
            continue;
        }

        // Tokenize
        while (command.length() > 0)
        {
            int position = command.find(" ");
            if (position != -1)
            {
                tokens.push_back(command.substr(0, position));
                command = command.substr(position + 1, command.length() - position - 1);
            }
            else
            {
                tokens.push_back(command);
                command = "";
            }
        }

        // Execute the command
        if (tokens[0] == "c")
        {
            debugging = false;
        }
        else if (tokens[0] == "help")
        {
            if (tokens.size() > 1)
            {
                cout << "Invalid amount of parameters. See \"help\" for help (jk)." << endl;
                continue;
            }
            cout << "help: displays this text" << endl;
            cout << "showregs: displays all cpu registers" << endl;
            cout << "setreg [register] [value]: sets the value of a general purpose register" << endl;
            cout << "bpa [address]: sets an arm mode execution breakpoint" << endl;
            cout << "bpt [address]: sets a thumb mode execution breakpoint" << endl;
            cout << "bpr [offset]: breakpoint on read" << endl;
            cout << "bpw [offset]: breakpoint on write" << endl;
            cout << "bpsvc [bios_call]: halt *after* executing a swi instruction with the given call number" << endl;
            cout << "cbp: remove all breakpoints" << endl;
            cout << "memdump [offset] [length]: displays memory in a formatted way" << endl;
            cout << "setmemb [offset] [byte]: writes a byte to a given memory address" << endl;
            cout << "setmemh [offset] [hword]: writes a hword to a given memory address" << endl;
            cout << "setmemw [offset] [word]: writes a word to a given memory address" << endl;
            cout << "dumpstck [count]: displays a given amount of stack entries" << endl;
            cout << "frame: run until the first line of the next frame gets rendered" << endl;
            cout << "c: continues execution" << endl;
            cout << endl;
        }
        else if (tokens[0] == "showregs")
        {
            arm_print_registers(arm);
        }
        else if (tokens[0] == "setreg")
        {
            uint reg;
            uint value;

            // Catch case of too many / few parameters
            if (tokens.size() != 3)
            {
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;
                continue;
            }

            // Parse parameters
            try
            {
                reg = take_word(tokens[1]);
                value = take_word(tokens[2]);
                if (reg > 0xF)
                {
                    throw new out_of_range("Parameter exceeds amount of registers");
                }
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }

            // Set register
            arm->SetGeneralRegister(reg, value);
        }
        else if (tokens[0] == "bpt" || tokens[0] == "bpa" || tokens[0] == "bpw" || tokens[0] == "bpr")
        {
            uint offset;
            ARM7Breakpoint* breakpoint;

            // Catch case of too many / few parameters
            if (tokens.size() != 2)
            {
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;
                continue;
            }

            breakpoint = new ARM7Breakpoint();

            // Parse parameter
            try
            {
                offset = take_word(tokens[1]);
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }

            // Create breakpoint object
            breakpoint->breakpoint_type = (tokens[0] == "bpt" || tokens[0] == "bpa") ? ARM7Breakpoint::ARM7BreakpointType::Execute : (tokens[0] == "bpw" ? ARM7Breakpoint::ARM7BreakpointType::Write : ARM7Breakpoint::ARM7BreakpointType::Read);
            breakpoint->concerned_address = offset;
            if (tokens[0] != "bpt" && tokens[0] != "bpa")
            {
                breakpoint->thumb_mode = tokens[0] == "bpt";
            }

            // Pass it to the processor
            arm->breakpoints.push_back(breakpoint);
        }
        else if (tokens[0] == "bpsvc")
        {
            // Catch too many / few parameters
            if (tokens.size() != 2)
            {
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;
                continue;
            }

            // Handle command
            try
            {
                ARM7Breakpoint* breakpoint = new ARM7Breakpoint();
                breakpoint->breakpoint_type = ARM7Breakpoint::ARM7BreakpointType::SVC;
                breakpoint->bios_call = take_byte(tokens[1]);
                arm->breakpoints.push_back(breakpoint);
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }
        }
        else if (tokens[0] == "cbp")
        {
            arm->breakpoints.clear();
        }
        else if (tokens[0] == "memdump")
        {
            uint offset;
            int count;

            // Catch too many / few parameters
            if (tokens.size() != 3)
            {
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;// use macro
                continue;
            }

            // Parse parameters
            try
            {
                offset = take_word(tokens[1]);
                count = take_word(tokens[2]);
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }

            // Display memory
            while (count != 0)
            {
                for (int i = 0; i < 16; i++)
                {
                    ubyte value = memory->ReadByte(offset);
                    cout << display_byte << (int)memory->ReadByte(offset) << " ";
                    if (--count == 0)
                    {
                        break;
                    } 
                    offset++;
                }
                cout << endl;
            }
        }
        else if (tokens[0] == "setmemb" || tokens[0] == "setmemh" || tokens[0] == "setmemw")
        {
            uint offset;

            // Catch case too many / few parameters
            if (tokens.size() != 3)
            { 
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;
                continue;
            }

            // Parse parameters and perform write
            try
            {
                offset = take_word(tokens[1]);
                switch (tokens[0][6])
                {
                case 'b':
                    memory->WriteByte(offset, take_byte(tokens[2]));
                    break;
                case 'h':
                    memory->WriteHWord(offset, take_hword(tokens[2]));
                    break;
                case 'w':
                    memory->WriteWord(offset, take_word(tokens[2]));
                    break;
                }
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }
        }
        else if (tokens[0] == "dumpstck")
        {
            uint count;

            // Catch too many / few parameters
            if (tokens.size() != 2)
            {
                cout << "Invalid amount of parameters. See \"help\" for help." << endl;
                continue;
            }

            // Parse parameter and dump
            try
            {
                uint stack_ptr = arm->GetGeneralRegister(13) + 4;
                count = take_word(tokens[1]);

                // Message
                cout << "Dumping stack from 0x" << display_word << stack_ptr << dec << " (r13 + 4):" << endl;

                // Actual dumping
                for (uint i = 0; i < count; i++)
                {
                    uint value = memory->ReadWord(stack_ptr + i * 4);
                    cout << "[0x" << display_word << stack_ptr << "+0x" << (i * 4) << "]: 0x" << 
                            display_word << value << dec << 
                            (((value >> 24) >= 8 && (value >> 24) <= 9) ? " (*)" : "") << /* mark entries which (could) point to the rom */
                            endl;
                }
            }
            catch (exception& e)
            {
                cout << e.what() << endl;
            }
        }
        else if (tokens[0] == "frame")
        {
            step_frame = true;
            debugging = false;
        }
        else
        {
            cout << "Uhh! I don't know that. Try \"help\" for help." << endl;
        }
    }
}

int main(int argc, char **argv)
{
    SDL_Event event;
	GBAMemory* memory;
	ARM7* arm;
    bool running = true;

    step_frame = false;

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

	arm = new ARM7(memory, true);

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
            debugger(arm, memory);
        }

        for (int i = 0; i < 280896; i++)
        {
            step(arm, memory);

            // Call debugger if we hit a breakpoint or the processor crashed
            if (arm->hit_breakpoint || arm->crashed)
            {
                debugger(arm, memory);
            }
            
            // Copy the finished frame to SDLs pixel buffer
            if (memory->video->render_scanline && memory->gba_io->vcount == 159)
            {
                if (step_frame)
                { 
                    step_frame = false; // do only break for the current frame
                    debugger(arm, memory);
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
