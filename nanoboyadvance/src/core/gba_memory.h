/*
* Copyright (C) 2015 Frederic Meyer
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

#pragma once

#include <iostream>
#include "common/types.h"
#include "gba_io.h"
#include "gba_interrupt.h"
#include "gba_video.h"
#include "gba_backup_flash.h"

using namespace std;

namespace NanoboyAdvance
{
    class GBAMemory
    {
        static const int tmr_cycles[4];
        static const int wsn_table[4];
        static const int wss0_table[2];
        static const int wss1_table[2];
        static const int wss2_table[2];
    
        u8 wram[0x40000];
        u8 iram[0x8000];
        u8 sram[0x10000];
        u8* bios;
        u8* rom;
        int rom_size;

        // DMA related (todo: enums have class)
        enum AddressControl
        {
            Increment = 0,
            Decrement = 1,
            Fixed = 2,
            IncrementAndReload = 3
        };
        
        // WAITCNT IO
        int ws_sram {0};
        int ws_first[3] {0, 0, 0};
        int ws_second[3] {0, 0, 0};
        bool gp_prefetch {false};

        // DMA (internal) IO
        u32 dma_src_int[4];
        u32 dma_dst_int[4];
        u32 dma_count_int[4];
        u32 dma_src[4];
        u32 dma_dst[4];
        u16 dma_count[4];
        AddressControl dma_dst_cntl[4];
        AddressControl dma_src_cntl[4];
        bool dma_repeat[4];
        bool dma_words[4];
        bool dma_gp_drq[4];
        int dma_time[4];
        bool dma_irq[4];
        bool dma_enable[4];
        
        // Timer IO
        u16 tmr_count[4] {0, 0, 0, 0};
        u16 tmr_reload[4];
        int tmr_clock[4];
        int tmr_ticks[4];
        bool tmr_enable[4];
        bool tmr_countup[4];
        bool tmr_irq[4];
    public:
        // Hardware / IO accessible through memory
        GBAInterrupt* interrupt;
        GBAVideo* video;
        GBABackup* backup;
        
        // Keypad IO
        u16 keyinput {256};
        
        // Timing related
        enum class AccessSize
        {
            Byte,
            Hword,
            Word
        };

        // Flags
        enum class GBAHaltState
        {
            None,
            Stop,
            Halt
        };
        GBAHaltState halt_state { GBAHaltState::None };
        bool intr_wait { false };
        bool intr_wait_mask { 0 };

        enum class GBASaveType
        {
            EEPROM,
            SRAM,
            FLASH64,
            FLASH128
        };
        GBASaveType save_type { GBASaveType::SRAM };

        // Schedules DMA
        void Step();

        // Cycle measurement methods
        int SequentialAccess(u32 offset, AccessSize size);
        int NonSequentialAccess(u32 offset, AccessSize size);
        
        // Read / Write access methods
        u8 ReadByte(u32 offset);
        u16 ReadHWord(u32 offset);
        u32 ReadWord(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);
        
        // Constructor and Destructor
        GBAMemory(string bios_file, string rom_file, string save_file);
        ~GBAMemory();
    };
}
