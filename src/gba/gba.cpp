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


#include "gba.h"
#include "audio/sdl_adapter.h"
#include "config/config.h"
#include <stdexcept>
#include <iostream>


using namespace std;
using namespace util;


namespace GBA
{
    // Defines the amount of cycles per frame
    const int GBA::FRAME_CYCLES = 280896;


    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Constructor, 1
    ///
    ///////////////////////////////////////////////////////////
    GBA::GBA(string rom_file, string save_file)
    {
        Memory::Init(rom_file, save_file);
        m_ARM.Init(true);

        // Rudimentary Audio setup
        SDL2AudioAdapter adapter;
        adapter.Init(&Memory::m_Audio);
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Constructor, 2
    ///
    ///////////////////////////////////////////////////////////
    GBA::GBA(string rom_file, string save_file, string bios_file)
    {
        u8* bios;
        size_t bios_size;

        if (!file::exists(bios_file))
            throw runtime_error("Cannot open BIOS file.");

        bios = file::read_data(bios_file);
        bios_size = file::get_size(bios_file);

        Memory::Init(rom_file, save_file, bios, bios_size);
        m_ARM.Init(false);

        // Rudimentary Audio setup
        SDL2AudioAdapter adapter;
        adapter.Init(&Memory::m_Audio);
    }

    GBA::~GBA()
    {
        Memory::Shutdown();
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Frame
    ///
    ///////////////////////////////////////////////////////////
    void GBA::Frame()
    {
        int total_cycles = FRAME_CYCLES * m_SpeedMultiplier;

        m_DidRender = false;

        for (int i = 0; i < total_cycles; ++i)
        {
            u32 requested_and_enabled = Interrupt::GetRequestedAndEnabled();

            // Only pause as long as (IE & IF) != 0
            if (Memory::m_HaltState != HALTSTATE_NONE && requested_and_enabled != 0)
            {
                // If IntrWait only resume if requested interrupt is encountered
                if (!Memory::m_IntrWait || (requested_and_enabled & Memory::m_IntrWaitMask) != 0)
                {
                    Memory::m_HaltState = HALTSTATE_NONE;
                    Memory::m_IntrWait = false;
                }
            }

            // Raise an IRQ if neccessary
            if (Interrupt::GetMasterEnable() && requested_and_enabled)
                m_ARM.RaiseIRQ();

            // Run the hardware components
            if (Memory::m_HaltState != HALTSTATE_STOP)
            {
                int forward_steps = 0;

                // Do next pending DMA transfer
                Memory::RunDMA();

                if (Memory::m_HaltState != HALTSTATE_HALT)
                {
                    m_ARM.cycles = 0;
                    m_ARM.Step();
                    forward_steps = m_ARM.cycles - 1;
                }

                for (int j = 0; j < forward_steps + 1; j++)
                {
                    bool overflow = false;
                    Memory::m_Timer[0].Step(overflow);
                    Memory::m_Timer[1].Step(overflow);
                    Memory::m_Timer[2].Step(overflow);
                    Memory::m_Timer[3].Step(overflow);

                    if (Memory::m_Video.m_WaitCycles == 0)
                    {
                        Memory::m_Video.Step();

                        if (Memory::m_Video.m_regenderScanline && (i / FRAME_CYCLES) == m_SpeedMultiplier - 1)
                        {
                            Memory::m_Video.Render();
                            m_DidRender = true;
                        }
                    }
                    else
                        Memory::m_Video.m_WaitCycles--;

                    if (((i + j) % 4) == 0)
                    {
                        Memory::m_Audio.m_QuadChannel[0].Step();
                        Memory::m_Audio.m_QuadChannel[1].Step();
                        Memory::m_Audio.m_WaveChannel.Step();
                    }

                    if (Memory::m_Audio.m_WaitCycles == 0)
                        Memory::m_Audio.Step();
                    else
                        Memory::m_Audio.m_WaitCycles--;
                }

                i += forward_steps;
            }
        }
    }

    void GBA::SetKeyState(Key key, bool pressed)
    {
        if (pressed)
            Memory::m_KeyInput &= ~(int)key;
        else
            Memory::m_KeyInput |= (int)key;
    }

    void GBA::SetSpeedUp(int multiplier)
    {
        m_SpeedMultiplier = multiplier;
    }

    u32* GBA::GetVideoBuffer()
    {
        return Memory::m_Video.m_OutputBuffer;
    }

    bool GBA::HasRendered()
    {
        return m_DidRender;
    }
}
