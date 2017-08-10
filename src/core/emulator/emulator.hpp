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
    public:
        Emulator(Config* config);
        ~Emulator();

        void reset();

        APU& getAPU();
        u16& getKeypad();
        void setKeyState(Key key, bool pressed);

        void reloadConfig();
        void loadGame(std::shared_ptr<Cartridge> cart);

        void runFrame();

    private:
        Config* config;

        // cycles until next PPU phase
        int  cycles_left;

        // subsystems
        PPU ppu;
        APU apu;
        Interrupt m_interrupt;

        // do not delete - needed for reference counting
        std::shared_ptr<Cartridge> cart;

        struct SystemMemory {
            u8 bios    [0x04000];
            u8 wram    [0x40000];
            u8 iram    [0x08000];
            u8 palette [0x00400];
            u8 oam     [0x00400];
            u8 vram    [0x18000];

            // Local copy (fast access)
            struct ROM {
                u8*    data;
                Save*  save;
                size_t size;
            } rom;

            u32 bios_opcode;

            //TODO: remove this hack
            u8 mmio[0x800];
        } memory;

        struct Registers {
            DMA   dma[4];
            Timer timer[4];

            u16 keyinput;

            struct Interrupt {
                u16 ie;
                u16 if_;
                u16 ime;
            } irq;

            SystemState haltcnt;
        } regs;

        // Memory Mapped I/O
        auto readMMIO (u32 address) -> u8;
        void writeMMIO(u32 address, u8 value);

        void runInternal(int cycles);

        // DMA emulation
        bool dma_running;
        int  dma_current;

        void dmaFindHBlank();
        void dmaFindVBlank();
        void dmaTransfer();
        void dmaTransferFIFO(int dma_id);

        // Timer emulation
        void timerStep(int cycles);
        void timerHandleFIFO(int timer_id, int times);
        void timerHandleOverflow(Timer& timer, int times);
        void timerIncrement(Timer& timer, int increment_count);
        void timerIncrementOnce(Timer& timer);

    protected:
        // memory bus implementation
        #include "memory/memory.hpp"

        void handleSWI(int number) final {};
    };
}
