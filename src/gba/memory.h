/*
* Copyright (C) 2016 Frederic Meyer
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
#include "util/types.h"
#include "iodef.h"
#include "interrupt.h"
#include "video.h"
#include "backup.h"

namespace NanoboyAdvance
{
    class GBAMemory
    {
        // DMA and Timer constants
        static const int dma_count_mask[4];
        static const int dma_dest_mask[4];
        static const int dma_source_mask[4];
        static const int tmr_cycles[4];

        // Waitstate constants
        static const int wsn_table[4];
        static const int wss0_table[2];
        static const int wss1_table[2];
        static const int wss2_table[2];

        // BIOS-stub for HLE-emulation.
        static const u8 hle_bios[0x40];

        enum class AddressControl
        {
            Increment = 0,
            Decrement = 1,
            Fixed = 2,
            Reload = 3
        };

        enum class StartTime
        {
            Immediate = 0,
            VBlank = 1,
            HBlank = 2,
            Special = 3
        };

        enum class TransferSize
        {
            HWord = 0,
            Word = 1
        };

        u8* rom;
        size_t rom_size;
        u8 bios[0x4000];
        u8 wram[0x40000];
        u8 iram[0x8000];

        // DMA IO
        struct DMA
        {
            u32 dest {0};
            u32 source {0};
            u16 count {0};
            u32 dest_int {0};
            u32 source_int {0};
            u16 count_int {0};
            AddressControl dest_control {AddressControl::Increment};
            AddressControl source_control {AddressControl::Increment};
            StartTime start_time {StartTime::Immediate};
            TransferSize size {TransferSize::HWord};
            bool repeat {false};
            bool gamepack_drq {false};
            bool interrupt {false};
            bool enable {false};
        } dma[4];
        int next_dma {0};

        // Timer IO
        struct Timer
        {
            u16 count {0};
            u16 reload {0};
            int clock {0};
            int ticks {0};
            bool enable {false};
            bool countup {false};
            bool interrupt {false};
        } timer[4];

        // Waitstate IO
        struct
        {
            int sram {0};
            int first[3] {0, 0, 0};
            int second[3] {0, 0, 0};
            bool prefetch {false};
        } waitstate;
    public:
        enum class AccessSize
        {
            Byte,
            Hword,
            Word
        };

        enum class HaltState
        {
            None,
            Stop,
            Halt
        };

        enum class SaveType
        {
            NONE,
            EEPROM,
            SRAM,
            FLASH64,
            FLASH128
        };

        GBAInterrupt* interrupt;
        GBAVideo* video;
        GBABackup* backup {NULL};
        SaveType save_type {SaveType::NONE};

        // Keypad IO
        u16 keyinput {0x3FF};
        
        HaltState halt_state {HaltState::None};
        bool intr_wait {false};
        bool intr_wait_mask {0};

        // DMA publics
        bool did_transfer {false};
        int dma_cycles {0};

        // Constructors and Destructor
        GBAMemory(std::string rom_file, std::string save_file);
        GBAMemory(std::string rom_file, std::string save_file, u8* bios, size_t bios_size);
        ~GBAMemory();

        // Update timer and DMA methods
        void RunTimer();
        void RunDMA();

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
    };
}
