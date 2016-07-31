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


#ifndef __NBA_MEMORY_H__
#define __NBA_MEMORY_H__


#include <iostream>
#include "util/types.h"
#include "iodef.h"
#include "interrupt.h"
#include "video.h"
#include "backup.h"


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    memory.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   GBAMemory
    /// \brief   Defines components of the vanilla memory.
    ///
    ///////////////////////////////////////////////////////////
    class GBAMemory
    {
    public:

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


        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   AddressControl
        ///
        /// Defines all possible address functionalities.
        ///
        ///////////////////////////////////////////////////////////
        enum class AddressControl
        {
            Increment   = 0,
            Decrement   = 1,
            Fixed       = 2,
            Reload      = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   StartTime
        ///
        /// Defines the start times for the DMA to copy memory.
        ///
        ///////////////////////////////////////////////////////////
        enum class StartTime
        {
            Immediate   = 0,
            VBlank      = 1,
            HBlank      = 2,
            Special     = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   TransferSize
        ///
        /// Defines the DMA transfer mode.
        ///
        ///////////////////////////////////////////////////////////
        enum class TransferSize
        {
            HWord       = 0,
            Word        = 1
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   AccessSize
        ///
        ///////////////////////////////////////////////////////////
        enum class AccessSize
        {
            Byte,
            Hword,
            Word
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   HaltState
        ///
        ///////////////////////////////////////////////////////////
        enum class HaltState
        {
            None,
            Stop,
            Halt
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   SaveType
        ///
        ///////////////////////////////////////////////////////////
        enum class SaveType
        {
            NONE,
            EEPROM,
            SRAM,
            FLASH64,
            FLASH128
        };


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  DMA
        /// \brief   Defines one DMA queue entry.
        ///
        ///////////////////////////////////////////////////////////
        struct DMA
        {
            u32 dest        {0};
            u32 source      {0};
            u16 count       {0};
            u32 dest_int    {0};
            u32 source_int  {0};
            u16 count_int   {0};
            AddressControl dest_control     {AddressControl::Increment};
            AddressControl source_control   {AddressControl::Increment};
            StartTime start_time            {StartTime::Immediate};
            TransferSize size               {TransferSize::HWord};
            bool repeat         {false};
            bool gamepack_drq   {false};
            bool interrupt      {false};
            bool enable         {false};
        } m_DMA[4];

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Timer
        /// \brief   Defines one DMA timer entry
        ///
        ///////////////////////////////////////////////////////////
        struct Timer
        {
            u16 count {0};
            u16 reload {0};
            int clock {0};
            int ticks {0};
            bool enable {false};
            bool countup {false};
            bool interrupt {false};
        } m_Timer[4];

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  waitstate
        ///
        ///////////////////////////////////////////////////////////
        struct waitstate
        {
            int sram        {0};
            int first[3]    {0, 0, 0};
            int second[3]   {0, 0, 0};
            bool prefetch   {false};
        } m_Waitstate;


    public:

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 1
        ///
        ///////////////////////////////////////////////////////////
        GBAMemory(std::string rom_file, std::string save_file);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 2
        ///
        ///////////////////////////////////////////////////////////
        GBAMemory(std::string rom_file, std::string save_file, u8* m_BIOS, size_t bios_size);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Destructor
        ///
        ///////////////////////////////////////////////////////////
        ~GBAMemory();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RunTimer
        ///
        ///////////////////////////////////////////////////////////
        void RunTimer();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RunDMA
        ///
        /// Processes all queued DMA entries.
        ///
        ///////////////////////////////////////////////////////////
        void RunDMA();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      SequentialAccess
        ///
        ///////////////////////////////////////////////////////////
        int SequentialAccess(u32 offset, AccessSize size);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      NonSequentialAccess
        ///
        ///////////////////////////////////////////////////////////
        int NonSequentialAccess(u32 offset, AccessSize size);

        
        // Read / Write access methods
        u8 ReadByte(u32 offset);
        u16 ReadHWord(u32 offset);
        u32 ReadWord(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);


    public:

        ///////////////////////////////////////////////////////////
        // Class members
        //
        ///////////////////////////////////////////////////////////
        u8* m_Rom;
        size_t m_RomSize;
        u8 m_BIOS[0x4000];
        u8 m_WRAM[0x40000];
        u8 m_IRAM[0x8000];
        GBAInterrupt* m_Interrupt;
        GBAVideo* m_Video;
        GBABackup* m_Backup         {NULL};
        SaveType m_SaveType         {SaveType::NONE};
        u16 m_KeyInput              {0x3FF};
        HaltState m_HaltState       {HaltState::None};
        bool m_IntrWait             {false};
        bool m_IntrWaitMask         {0};
        bool m_DidTransfer          {false};
        int m_DMACycles             {0};
    };
}


#endif  // __NBA_MEMORY_H__
