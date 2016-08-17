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
    private:

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
        /// Defines all possible DMA address functionalities.
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
        /// Defines the DMA transfer size.
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
        /// \enum   SaveType
        ///
        /// Defined cartridge backup/save types.
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
        /// \brief   Defines one DMA channel.
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
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Timer
        /// \brief   Defines one timer.
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
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \struct  Waitstate
        ///
        /// Holds waitstate configuration info.
        ///
        ///////////////////////////////////////////////////////////
        struct Waitstate
        {
            int sram        {0};
            int first[3]    {0, 0, 0};
            int second[3]   {0, 0, 0};
            bool prefetch   {false};
        };


    public:

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   AccessSize
        ///
        /// Defines different memory access sizes.
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
        /// Defines system execution states.
        ///
        ///////////////////////////////////////////////////////////
        enum class HaltState
        {
            None,
            Stop,
            Halt
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 1
        /// \param   rom_file         Path to the game dump.
        /// \param   save_file        Path to the save file.
        ///
        ///////////////////////////////////////////////////////////
        GBAMemory(std::string rom_file, std::string save_file);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 2
        /// \param   rom_file         Path to the game dump.
        /// \param   save_file        Path to the save file.
        /// \param   bios             BIOS-rom buffer.
        /// \param   bios_size        Size of the BIOS-rom buffer.
        ///
        ///////////////////////////////////////////////////////////
        GBAMemory(std::string rom_file, std::string save_file, u8* bios, size_t bios_size);

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
        /// Updates all timers and timer-driven audio.
        ///
        ///////////////////////////////////////////////////////////
        void RunTimer();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RunDMA
        ///
        /// Processes all DMA channels.
        ///
        ///////////////////////////////////////////////////////////
        void RunDMA();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      SequentialAccess
        ///
        /// Calculates sequential access cycles for a given access.
        ///
        /// \param    offset  The address that is accessed.
        /// \param    size    The access size (byte/hword/word).
        /// \returns  The amount of cycles.
        ///
        ///////////////////////////////////////////////////////////
        int SequentialAccess(u32 offset, AccessSize size);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      NonSequentialAccess
        ///
        /// Calculates non-sequential access cycles for a given access.
        ///
        /// \param    offset  The address that is accessed.
        /// \param    size    The access size (byte/hword/word).
        /// \returns  The amount of cycles.
        ///
        ///////////////////////////////////////////////////////////
        int NonSequentialAccess(u32 offset, AccessSize size);

        
        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      ReadByte
        ///
        /// Reads one byte from a given address.
        ///
        /// \param    offset  The address to read from.
        /// \returns  The byte being read.
        ///
        ///////////////////////////////////////////////////////////
        u8 ReadByte(u32 offset);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      ReadHWord
        ///
        /// Reads one hword from a given address.
        ///
        /// \param    offset  The address to read from.
        /// \returns  The hword being read.
        ///
        ///////////////////////////////////////////////////////////
        u16 ReadHWord(u32 offset);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      ReadWord
        ///
        /// Reads one word from a given address.
        ///
        /// \param    offset  The address to read from.
        /// \returns  The word being read.
        ///
        ///////////////////////////////////////////////////////////
        u32 ReadWord(u32 offset);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      WriteByte
        ///
        /// Writes one byte to a given address.
        ///
        /// \param  offset  The address to write to.
        /// \param  value   The byte to write to the address.
        ///
        ///////////////////////////////////////////////////////////
        void WriteByte(u32 offset, u8 value);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      WriteHWord
        ///
        /// Writes one hword to a given address.
        ///
        /// \param  offset  The address to write to.
        /// \param  value   The hword to write to the address.
        ///
        ///////////////////////////////////////////////////////////
        void WriteHWord(u32 offset, u16 value);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 2th, 2016
        /// \fn      WriteWord
        ///
        /// Writes one word to a given address.
        ///
        /// \param  offset  The address to write to.
        /// \param  value   The word to write to the address.
        ///
        ///////////////////////////////////////////////////////////
        void WriteWord(u32 offset, u32 value);


    private:

        ///////////////////////////////////////////////////////////
        // Class members (Memory)
        //
        ///////////////////////////////////////////////////////////
        u8* m_ROM;
        size_t m_ROMSize;
        u8 m_BIOS[0x4000];
        u8 m_WRAM[0x40000];
        u8 m_IRAM[0x8000];
        GBABackup* m_Backup         {NULL};
        SaveType m_SaveType         {SaveType::NONE};

        ///////////////////////////////////////////////////////////
        // Class members (DMA, Timer, Waitstate)
        //
        ///////////////////////////////////////////////////////////
        struct DMA  m_DMA[4];
        struct Timer m_Timer[4];
        struct Waitstate m_Waitstate;
        u32 m_SOUNDBIAS {0}; // preliminary SOUNDBIAS implementation.
    public:
        ///////////////////////////////////////////////////////////
        // Class members (Interrupts)
        //
        ///////////////////////////////////////////////////////////
        GBAInterrupt* m_Interrupt;
        HaltState m_HaltState       {HaltState::None};
        bool m_IntrWait             {false};
        bool m_IntrWaitMask         {0};

        ///////////////////////////////////////////////////////////
        // Class members (misc.)
        //
        ///////////////////////////////////////////////////////////
        GBAVideo* m_Video;
        bool m_DidTransfer          {false};
        int m_DMACycles             {0};
        u16 m_KeyInput              {0x3FF};
    };
}


#endif  // __NBA_MEMORY_H__
