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


using namespace std;


namespace NanoboyAdvance
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
        //m_Memory = new GBAMemory(rom_file, save_file);
        //m_ARM = new ARM7(m_Memory, true);
        m_Memory.Init(rom_file, save_file);
        m_ARM.Init(&m_Memory, true);
        m_Memory.m_Video.SetupComposer(&m_Composer);
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

        if (!File::Exists(bios_file))
            throw runtime_error("Cannot open BIOS file.");

        bios = File::ReadFile(bios_file);
        bios_size = File::GetFileSize(bios_file);

        m_Memory.Init(rom_file, save_file, bios, bios_size);
        m_ARM.Init(&m_Memory, false);
        m_Memory.m_Video.SetupComposer(&m_Composer);
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Destructor
    ///
    ///////////////////////////////////////////////////////////
    GBA::~GBA()
    {
        //delete m_ARM;
        //delete m_Memory;
        //delete m_Composer;
    }


    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Frame
    ///
    ///////////////////////////////////////////////////////////
    void GBA::Frame()
    {
        m_DidRender = false;

        for (int i = 0; i < FRAME_CYCLES * m_SpeedMultiplier; i++)
        {
            u32 interrupts = m_Memory.m_Interrupt.ie & m_Memory.m_Interrupt.if_;
            
            // Only pause as long as (IE & IF) != 0
            if (m_Memory.m_HaltState != GBAMemory::HaltState::None && interrupts != 0)
            {
                // If IntrWait only resume if requested interrupt is encountered
                if (!m_Memory.m_IntrWait || (interrupts & m_Memory.m_IntrWaitMask) != 0)
                {
                    m_Memory.m_HaltState = GBAMemory::HaltState::None;
                    m_Memory.m_IntrWait = false;
                }
            }

            // Raise an IRQ if neccessary
            //if (m_Memory.m_Interrupt.ime && interrupts)
            //    m_ARM.RaiseIRQ();

            // Run the hardware components
            if (m_Memory.m_HaltState != GBAMemory::HaltState::Stop)
            {
                int forward_steps = 0;

                // Do next pending DMA transfer
                //m_Memory.RunDMA();

                if (m_Memory.m_HaltState != GBAMemory::HaltState::Halt)
                {
                    m_ARM.cycles = 0;
                    m_ARM.Step();
                    forward_steps = m_ARM.cycles - 1;
                }

//                m_Memory.m_Video.Step();
                for (int j = 0; j < forward_steps + 1; j++) 
                {
                    //m_Memory.m_Video.Step();
                    //m_Memory.m_Audio.Step();
                    //m_Memory.RunTimer();

                    /*if (m_Memory.m_Video.m_RenderScanline && (i / FRAME_CYCLES) == m_SpeedMultiplier - 1)
                    {
                        m_Memory.m_Video.Render();
                        m_Composer.Scanline();
                        m_DidRender = true;
                    }*/
                }

                i += forward_steps;
            }
        }

        m_Composer.Frame();
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      SetKeyState
    ///
    ///////////////////////////////////////////////////////////
    void GBA::SetKeyState(Key key, bool pressed)
    {
        if (pressed)
            m_Memory.m_KeyInput &= ~(int)key;
        else
            m_Memory.m_KeyInput |= (int)key;
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      SetSpeedUp
    ///
    ///////////////////////////////////////////////////////////
    void GBA::SetSpeedUp(int multiplier)
    {
        m_SpeedMultiplier = multiplier;
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      GetVideoBuffer
    ///
    ///////////////////////////////////////////////////////////
    u32* GBA::GetVideoBuffer()
    {
        return m_Composer.m_OutputBuffer;
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      HasRendered
    ///
    ///////////////////////////////////////////////////////////
    bool GBA::HasRendered()
    {
        return m_DidRender;
    }
}
