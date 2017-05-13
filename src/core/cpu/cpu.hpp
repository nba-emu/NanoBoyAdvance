/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <string>
#include "../arm/arm.hpp"
#include "enums.hpp"
#include "interrupt.hpp"
#include "../config.hpp"
#include "../ppu/ppu.hpp"
#include "../apu/apu.hpp"
#include "../cart/cart_backup.hpp"
#define CPU_INCLUDE

namespace GameBoyAdvance {
    
    class CPU : public ARM {
    private:
        Config* m_config;
        
        u8*    m_rom;
        size_t m_rom_size;
        CartBackup* m_backup {nullptr};

        u8 m_bios[0x4000];
        u8 m_wram[0x40000];
        u8 m_iram[0x8000];
        u8 m_pal[0x400];
        u8 m_oam[0x400];
        u8 m_vram[0x18000];

        u8 m_mmio[0x800];

        u32 bios_opcode;
        
        #include "io.hpp"

        PPU m_ppu;
        APU m_apu;
        Interrupt m_interrupt;

        int  m_cycles;
        bool m_dma_active;
        int  m_current_dma;

        void load_game();
        
        auto read_mmio (u32 address) -> u8;
        void write_mmio(u32 address, u8 value);

        void run_for(int cycles);

        void timer_step(int cycles);
        void timer_fifo(int timer_id, int times);
        void timer_overflow(IO::Timer& timer, int times);
        void timer_increment(IO::Timer& timer, int increment_count);
        void timer_increment_once(IO::Timer& timer);
        
        void dma_hblank();
        void dma_vblank();
        void dma_transfer();
        void dma_fill_fifo(int dma_id);

        static constexpr int s_mem_cycles8_16[16] = {
            1, 1, 3, 1, 1, 1, 1, 1, 5, 5, 1, 1, 1, 1, 5, 1
        };

        static constexpr int s_mem_cycles32[16] = {
            1, 1, 6, 1, 1, 2, 2, 1, 8, 8, 1, 1, 1, 1, 5, 1
        };

    public:
        CPU(Config* config);
        ~CPU();

        void reset();

        APU& get_apu();
        u16& get_keypad();

        void load_config();
        void load_game(std::string rom_file, std::string save_file);
        
        void frame();
    protected:

        #include "memory.hpp"

        void software_interrupt(int number) final {};
    };
}

#undef CPU_INCLUDE
