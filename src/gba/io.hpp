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

#ifdef CPU_INCLUDE

struct IO {
    struct DMA {
        int id;

        bool enable;
        bool repeat;
        bool interrupt;
        bool gamepak;

        u16 length;
        u32 dst_addr;
        u32 src_addr;
        DMAControl dst_cntl;
        DMAControl src_cntl;
        DMATime time;
        DMASize size;

        struct Internal {
            u16 length;
            u32 dst_addr;
            u32 src_addr;
        } internal;

        // !!hacked!! much ouch
        bool* dma_active;
        int*  current_dma;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } dma[4];

    struct Timer {
        int id;

        struct Control {
            int frequency;
            bool cascade;
            bool interrupt;
            bool enable;
        } control;

        int ticks;
        u16 reload;
        u16 counter;

        void reset();
        auto read(int offset) -> u8;
        void write(int offset, u8 value);
    } timer[4];

    // JOYPAD
    u16 keyinput;

    // INTERRUPT CONTROL
    struct Interrupt {
        u16 enable;
        u16 request;
        u16 master_enable;

        void reset();
        // todo: place read/write methods here!
    } interrupt;

    SystemState haltcnt;
} m_io;

#endif
