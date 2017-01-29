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
    constexpr int cpu::m_mem_cycles8_16[16];
    constexpr int cpu::m_mem_cycles32[16];
    constexpr cpu::read_func cpu::m_read_table[16];
    constexpr cpu::write_func cpu::m_write_table[16];

    cpu::cpu()
    {
        reset();

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

        // reset IO-registers
        m_io.keyinput = 0x3FF;
        m_io.interrupt.reset();
        m_io.haltcnt = SYSTEM_RUN;
        for (int i = 0; i < 4; i++)
        {
            m_io.timer[i].id = i;
            m_io.timer[i].reset();
        }

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

    ppu& cpu::get_ppu()
    {
        return m_ppu;
    }

    u16& cpu::get_keypad()
    {
        return m_io.keyinput;
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

    // TODO: should be in ARMigo core and replace step-method maybe
    void cpu::run_for(int cycles)
    {
        int cycles_previous;

        m_cycles += cycles;

        while (m_cycles >= 0)
        {
            u32 requested_and_enabled = m_io.interrupt.request & m_io.interrupt.enable;

            if (m_io.haltcnt == SYSTEM_HALT && requested_and_enabled)
            {
                m_io.haltcnt = SYSTEM_RUN;
            }

            if (m_io.haltcnt == SYSTEM_RUN)
            {
                cycles_previous = m_cycles;

                if (m_io.interrupt.master_enable && requested_and_enabled)
                {
                    raise_irq();
                }

                step();
                timer_step(cycles_previous - m_cycles);
            }
            else
            {
                timer_step(m_cycles);
                m_cycles = 0;
                return;
            }
        }
    }
}
