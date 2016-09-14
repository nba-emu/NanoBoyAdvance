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
#include "common/types.h"
#include "interrupt.h"
#include "video/video.h"
#include "backup/backup.h"
#include "audio/audio.h"


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    memory.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   Memory
    /// \brief   Defines components of the vanilla memory.
    ///
    ///////////////////////////////////////////////////////////
    class Memory
    {
    private:
        typedef u8   (Memory::*ReadFunction) (u32 address);
        typedef void (Memory::*WriteFunction)(u32 address, u8 value);

        u8 ReadBIOS(u32 address);
        u8 ReadWRAM(u32 address);
        u8 ReadIRAM(u32 address);
        u8 ReadIO(u32 address);
        u8 ReadPAL(u32 address);
        u8 ReadVRAM(u32 address);
        u8 ReadOAM(u32 address);
        u8 ReadROM(u32 address);
        u8 ReadPageE(u32 address);
        u8 ReadInvalid(u32 address);

        void WriteWRAM(u32 address, u8 value);
        void WriteIRAM(u32 address, u8 value);
        void WriteIO(u32 address, u8 value);
        void WritePAL(u32 address, u8 value);
        void WriteVRAM(u32 address, u8 value);
        void WriteOAM(u32 address, u8 value);
        void WritePageE(u32 address, u8 value);
        void WriteInvalid(u32 address, u8 value);

        // Memory Read Byte methods
        static constexpr ReadFunction READ_TABLE[16] = {
            &ReadBIOS,
            &Memory::ReadInvalid,
            &Memory::ReadWRAM,
            &Memory::ReadIRAM,
            &Memory::ReadIO,
            &Memory::ReadPAL,
            &Memory::ReadVRAM,
            &Memory::ReadOAM,
            &Memory::ReadROM,
            &Memory::ReadROM,
            &Memory::ReadInvalid,
            &Memory::ReadInvalid,
            &Memory::ReadInvalid,
            &Memory::ReadInvalid,
            &Memory::ReadPageE,
            &Memory::ReadInvalid
        };

        // Memory Write Byte methods
        static constexpr WriteFunction WRITE_TABLE[16] = {
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WriteWRAM,
            &Memory::WriteIRAM,
            &Memory::WriteIO,
            &Memory::WritePAL,
            &Memory::WriteVRAM,
            &Memory::WriteOAM,
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WriteInvalid,
            &Memory::WritePageE,
            &Memory::WriteInvalid
        };

        // DMA and Timer constants
        static constexpr int DMA_COUNT_MASK[4] =
            { 0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF };
        static constexpr int DMA_DEST_MASK[4] =
            { 0x7FFFFFF, 0x7FFFFFF, 0x7FFFFFF, 0xFFFFFFF };
        static constexpr int DMA_SOURCE_MASK[4] =
            { 0x7FFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF };
        static constexpr int TMR_CYCLES[4] =
            { 1, 64, 256, 1024 };

        // Waitstate constants
        static constexpr int WSN_TABLE[4] = {4, 3, 2, 8};
        static constexpr int WSS0_TABLE[2] = {2, 1};
        static constexpr int WSS1_TABLE[2] = {4, 1};
        static constexpr int WSS2_TABLE[2] = {8, 1};

        // BIOS-stub for HLE-emulation.
        static const u8 HLE_BIOS[0x40];


        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   AddressControl
        ///
        /// Defines all possible DMA address functionalities.
        ///
        ///////////////////////////////////////////////////////////
        enum AddressControl
        {
            DMA_INCREMENT   = 0,
            DMA_DECREMENT   = 1,
            DMA_FIXED       = 2,
            DMA_RELOAD      = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   StartTime
        ///
        /// Defines the start times for the DMA to copy memory.
        ///
        ///////////////////////////////////////////////////////////
        enum StartTime
        {
            DMA_IMMEDIATE   = 0,
            DMA_VBLANK      = 1,
            DMA_HBLANK      = 2,
            DMA_SPECIAL     = 3
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   TransferSize
        ///
        /// Defines the DMA transfer size.
        ///
        ///////////////////////////////////////////////////////////
        enum TransferSize
        {
            DMA_HWORD       = 0,
            DMA_WORD        = 1
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   SaveType
        ///
        /// Defined cartridge backup/save types.
        ///
        ///////////////////////////////////////////////////////////
        enum SaveType
        {
            SAVE_NONE,
            SAVE_EEPROM,
            SAVE_SRAM,
            SAVE_FLASH64,
            SAVE_FLASH128
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
            AddressControl  dest_control     { DMA_INCREMENT };
            AddressControl  source_control   { DMA_INCREMENT };
            StartTime       start_time       { DMA_IMMEDIATE };
            TransferSize    size             { DMA_HWORD };
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
            u16 count  {0};
            u16 reload {0};
            int clock  {0};
            int ticks  {0};
            bool enable    {false};
            bool countup   {false};
            bool interrupt {false};
            bool overflow  {false};
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
        enum AccessSize
        {
            ACCESS_BYTE,
            ACCESS_HWORD,
            ACCESS_WORD
        };

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   HaltState
        ///
        /// Defines system execution states.
        ///
        ///////////////////////////////////////////////////////////
        enum HaltState
        {
            HALTSTATE_NONE,
            HALTSTATE_STOP,
            HALTSTATE_HALT
        };

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 1
        /// \param   rom_file         Path to the game dump.
        /// \param   save_file        Path to the save file.
        ///
        ///////////////////////////////////////////////////////////
        void Init(std::string rom_file, std::string save_file);

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
        void Init(std::string rom_file, std::string save_file, u8* bios, size_t bios_size);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Destructor
        ///
        ///////////////////////////////////////////////////////////
        ~Memory();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      RunTimer
        /// \brief   Updates all timers and timer-driven audio.
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
        Backup* m_Backup            {NULL};
        SaveType m_SaveType         {SAVE_NONE};

        ///////////////////////////////////////////////////////////
        // Class members (DMA, Timer, Waitstate, Audio)
        //
        ///////////////////////////////////////////////////////////
        struct DMA       m_DMA[4];
        struct Timer     m_Timer[4];
        struct Waitstate m_Waitstate;
    public:
        ///////////////////////////////////////////////////////////
        // Class members (Interrupts)
        //
        ///////////////////////////////////////////////////////////
        Interrupt m_Interrupt;
        HaltState    m_HaltState       {HALTSTATE_NONE};
        bool         m_IntrWait        {false};
        bool         m_IntrWaitMask    {0};

        ///////////////////////////////////////////////////////////
        // Class members (misc.)
        //
        ///////////////////////////////////////////////////////////
        Video m_Video;
        Audio m_Audio;
        bool m_DidTransfer          {false};
        int m_DMACycles             {0};
        u16 m_KeyInput              {0x3FF};
    };
}


#endif  // __NBA_MEMORY_H__
