///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
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

#include <cstring>
#include <stdexcept>
#include "cpu.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    constexpr cpu::read_func cpu::m_read_table[16];
    constexpr cpu::write_func cpu::m_write_table[16];

    cpu::cpu()
    {
        reset();

        // reset JOYPAD
        m_io.keyinput = 0x3FF;

        // reset interrupt controller
        m_io.interrupt.enable = 0;
        m_io.interrupt.request = 0;
        m_io.interrupt.master_enable = 0;

        // setup interrupt controller
        m_interrupt.set_flag_register(&m_io.interrupt.request);

        // feed PPU with important data.
        m_ppu.set_memory(m_pal, m_oam, m_vram);
        m_ppu.set_interrupt(&m_interrupt);
    }

    void cpu::reset()
    {
        arm::reset();
        m_ppu.reset();

        // clear out all memory
        memset(m_wram, 0, 0x40000);
        memset(m_iram, 0, 0x8000);
        memset(m_pal, 0, 0x400);
        memset(m_oam, 0, 0x400);
        memset(m_vram, 0, 0x18000);

        set_hle(false);

        //if (m_hle)
        {
            m_reg[15] = 0x08000000;
            m_reg[13] = 0x03007F00;
            m_bank[BANK_SVC][BANK_R13] = 0x03007FE0;
            m_bank[BANK_IRQ][BANK_R13] = 0x03007FA0;
            refill_pipeline();
        }
    }

    u16& cpu::get_keypad()
    {
        return m_io.keyinput;
    }

    u32* cpu::get_framebuffer()
    {
        return m_ppu.get_framebuffer(); // super eh...
    }

    void cpu::set_bios(u8* data, size_t size)
    {
        if (size <= sizeof(m_bios))
        {
            memcpy(m_bios, data, size);
            return;
        }
        throw std::runtime_error("bios file is too big.");
    }

    void cpu::set_game(u8* data, size_t size)
    {
        m_rom = data;
        m_rom_size = size;
    }

    void cpu::frame()
    {
        // 160 visible lines, alternating SCANLINE and HBLANK.
        for (int line = 0; line < 160; line++)
        {
            m_ppu.scanline();
            run_for(960);

            m_ppu.hblank();
            run_for(272);

            m_ppu.next_line();
        }

        // 68 invisible lines, VBLANK period.
        m_ppu.vblank();
        for (int line = 0; line < 68; line++)
        {
            run_for(1232);
            m_ppu.next_line();
        }
    }

    // TODO: should be in ARMigo core and replace step-method
    void cpu::run_for(int cycles)
    {
        // assumes IPC of 1/8 for now.
        for (int cycle = 0; cycle < cycles; cycle += 8)
        {
            if (m_io.interrupt.master_enable && ((m_io.interrupt.enable & m_io.interrupt.request) != 0))
            {
                raise_irq();
            }
            step();
        }
    }
}
