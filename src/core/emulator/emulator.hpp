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

#include "enums.hpp"
#include "interrupt.hpp"
#include "dma/regs.hpp"
#include "timer/regs.hpp"
#include "../config.hpp"
#include "../arm/arm.hpp"
#include "../ppu/ppu.hpp"
#include "../apu/apu.hpp"
#include "../cart/cartridge.hpp"

namespace GameBoyAdvance {

    class Emulator : private ARM {

    private:
        Config* m_config;

        // do not delete - needed for reference counting
        std::shared_ptr<Cartridge> cart;

        struct SystemMemory {
            u8 bios    [0x4000 ];
            u8 wram    [0x40000];
            u8 iram    [0x8000 ];
            u8 palette [0x400  ];
            u8 oam     [0x400  ];
            u8 vram    [0x18000];

            // Local copy (fast access)
            struct ROM {
                u8*    data;
                Save*  save;
                size_t size;
            } rom;
        } memory;

        //TODO: remove this hack
        u8 m_mmio[0x800];

        u32 bios_opcode;

        #include "io.hpp"

        // Subsystems
        PPU m_ppu;
        APU m_apu;
        Interrupt m_interrupt;

        int  m_cycles;
        bool m_dma_active;
        int  m_current_dma;

        auto read_mmio (u32 address) -> u8;
        void write_mmio(u32 address, u8 value);

        void run_for(int cycles);

        void timer_step(int cycles);
        void timer_fifo(int timer_id, int times);
        void timer_overflow(Timer& timer, int times);
        void timer_increment(Timer& timer, int increment_count);
        void timer_increment_once(Timer& timer);

        void dma_hblank();
        void dma_vblank();
        void dma_transfer();
        void dma_fill_fifo(int dma_id);

    public:
        Emulator(Config* config);
        ~Emulator();

        void reset();

        APU& get_apu();
        u16& get_keypad();

        void load_config();
        void load_game(std::shared_ptr<Cartridge> cart);

        void frame();
    protected:

        #include "memory.hpp"

        void software_interrupt(int number) final {};
    };
}
