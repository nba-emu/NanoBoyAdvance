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

#pragma once

#include "arm/arm.hpp"
#include "ppu/ppu.hpp"
#define CPU_INCLUDE

using namespace armigo;

namespace gba
{
    class cpu : public arm
    {
    private:
        typedef u8 (cpu::*read_func)(u32 address);
        typedef void (cpu::*write_func)(u32 address, u8 value);

        u8* m_rom;
        size_t m_rom_size;
        u8 m_bios[0x4000];
        u8 m_wram[0x40000];
        u8 m_iram[0x8000];
        u8 m_pal[0x400];
        u8 m_oam[0x400];
        u8 m_vram[0x18000];

        #include "io.hpp"

        ppu m_ppu;

        u8 read_bios(u32 address);
        u8 read_wram(u32 address);
        u8 read_iram(u32 address);
        u8 read_mmio(u32 address);
        u8 read_pal(u32 address);
        u8 read_vram(u32 address);
        u8 read_oam(u32 address);
        u8 read_rom(u32 address);
        u8 read_invalid(u32 address);

        void write_wram(u32 address, u8 value);
        void write_iram(u32 address, u8 value);
        void write_mmio(u32 address, u8 value);
        void write_pal(u32 address, u8 value);
        void write_vram(u32 address, u8 value);
        void write_oam(u32 address, u8 value);
        void write_invalid(u32 address, u8 value);

        static constexpr read_func m_read_table[16] = {
            &cpu::read_bios,
            &cpu::read_invalid,
            &cpu::read_wram,
            &cpu::read_iram,
            &cpu::read_mmio,
            &cpu::read_pal,
            &cpu::read_vram,
            &cpu::read_oam,
            &cpu::read_rom,
            &cpu::read_rom,
            &cpu::read_invalid,
            &cpu::read_invalid,
            &cpu::read_invalid,
            &cpu::read_invalid,
            &cpu::read_invalid,
            &cpu::read_invalid
        };

        static constexpr write_func m_write_table[16] = {
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_wram,
            &cpu::write_iram,
            &cpu::write_mmio,
            &cpu::write_pal,
            &cpu::write_vram,
            &cpu::write_oam,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid,
            &cpu::write_invalid
        };

    public:
        cpu();

        void reset();

        u16& get_keypad();
        u32* get_framebuffer(); // eh..
        void set_bios(u8* data, size_t size);
        void set_game(u8* data, size_t size);

        void frame();
    protected:
        u8 bus_read_byte(u32 address) final;
        u16 bus_read_hword(u32 address) final;
        u32 bus_read_word(u32 address) final;
        void bus_write_byte(u32 address, u8 value) final;
        void bus_write_hword(u32 address, u16 value) final;
        void bus_write_word(u32 address, u32 value) final;

        void software_interrupt(int number) final;
    };
}

#undef CPU_INCLUDE
