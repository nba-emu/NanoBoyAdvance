/*
* Copyright (C) 2016 Frederic Meyer
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

#include "gba.h"

using namespace std;

namespace NanoboyAdvance
{
    const int GBA::FRAME_CYCLES = 280896;

    GBA::GBA(string rom_file, string save_file)
    {
        memory = new GBAMemory(rom_file, save_file);
        arm = new ARM7(memory, true);
    }

    GBA::GBA(string rom_file, string save_file, string bios_file)
    {
        u8* bios;
        size_t bios_size;

        if (!File::Exists(bios_file))
            throw new runtime_error("Cannot open BIOS file.");

        bios = File::ReadFile(bios_file);
        bios_size = File::GetFileSize(bios_file);

        memory = new GBAMemory(rom_file, save_file, bios, bios_size);
        arm = new ARM7(memory, false);
    }

    GBA::~GBA()
    {
        delete arm;
        delete memory;
    }

    void GBA::Frame()
    {
        u32* buffer = (u32*)malloc(240 * 160 * sizeof(u32));

        for (int i = 0; i < FRAME_CYCLES * speed_multiplier; i++)
        {
            u32 interrupts = memory->interrupt->ie & memory->interrupt->if_;
            
            // Only pause as long as (IE & IF) != 0
            if (memory->halt_state != GBAMemory::HaltState::None && interrupts != 0)
            {
                // If IntrWait only resume if requested interrupt is encountered
                if (!memory->intr_wait || (interrupts & memory->intr_wait_mask) != 0)
                {
                    memory->halt_state = GBAMemory::HaltState::None;
                    memory->intr_wait = false;
                }
            }

            // Raise an IRQ if neccessary
            if (memory->interrupt->ime && interrupts)
                arm->RaiseIRQ();

            // Run the hardware components
            if (memory->halt_state != GBAMemory::HaltState::Stop)
            {
                int forward_steps = 0;

                // Do next pending DMA transfer
                memory->RunDMA();

                if (memory->halt_state != GBAMemory::HaltState::Halt)
                {
                    arm->cycles = 0;
                    arm->Step();
                    forward_steps = arm->cycles - 1;
                }

                for (int j = 0; j < forward_steps + 1; j++) 
                {
                    memory->video->Step();
                    memory->RunTimer();

                    if (memory->video->render_scanline && (i / FRAME_CYCLES) == speed_multiplier - 1)
                    {
                        int y = memory->video->vcount;
                        memory->video->Render(y);
                        memcpy(&pixel_buffer[y * 240], &memory->video->buffer[y * 240], 240 * sizeof(u32));
                    }
                }

                i += forward_steps;
            }
        }
    }

    void GBA::SetKeyState(Key key, bool pressed)
    {
        if (pressed)
            memory->keyinput &= ~(int)key;
        else
            memory->keyinput |= (int)key;
    }

    void GBA::SetSpeedUp(int multiplier)
    {
        speed_multiplier = multiplier;
    }
};