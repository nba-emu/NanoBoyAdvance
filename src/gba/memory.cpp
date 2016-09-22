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


#include "memory.h"
#include "common/log.h"
#include "common/file.h"
#include "backup/flash.h"
#include "backup/sram.h"
#include <stdexcept>
#include <cstring>


using namespace std;
using namespace Common;


namespace GBA
{
    constexpr Memory::ReadFunction  Memory::READ_TABLE[16];
    constexpr Memory::WriteFunction Memory::WRITE_TABLE[16];

    constexpr int Memory::DMA_COUNT_MASK[4];
    constexpr int Memory::DMA_DEST_MASK[4];
    constexpr int Memory::DMA_SOURCE_MASK[4];
    constexpr int Memory::TMR_CYCLES[4];

    constexpr int Memory::WSN_TABLE[4];
    constexpr int Memory::WSS0_TABLE[2];
    constexpr int Memory::WSS1_TABLE[2];
    constexpr int Memory::WSS2_TABLE[2];

    const u8 Memory::HLE_BIOS[0x40] = {
        0x06, 0x00, 0x00, 0xEA, 0x00, 0x00, 0xA0, 0xE1,
        0x00, 0x00, 0xA0, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
        0x00, 0x00, 0xA0, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
        0x01, 0x00, 0x00, 0xEA, 0x00, 0x00, 0xA0, 0xE1,
        0x02, 0xF3, 0xA0, 0xE3, 0x0F, 0x50, 0x2D, 0xE9,
        0x01, 0x03, 0xA0, 0xE3, 0x00, 0xE0, 0x8F, 0xE2,
        0x04, 0xF0, 0x10, 0xE5, 0x0F, 0x50, 0xBD, 0xE8,
        0x04, 0xF0, 0x5E, 0xE2, 0x00, 0x00, 0xA0, 0xE1
    };


    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     Constructor, 1
    ///
    ///////////////////////////////////////////////////////////
    void Memory::Init(string rom_file, string save_file)
    {
        Init(rom_file, save_file, nullptr, 0);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     Constructor, 2
    ///
    ///////////////////////////////////////////////////////////
    void Memory::Init(string rom_file, string save_file, u8* bios, size_t bios_size)
    {
        // Init memory buffers
        memset(this->m_BIOS, 0, 0x4000);
        memset(m_WRAM, 0, 0x40000);
        memset(m_IRAM, 0, 0x8000);

        // Load BIOS memory
        if (bios != nullptr)
        {
            if (bios_size > 0x4000)
                throw runtime_error("BIOS file is too big.");
            memcpy(this->m_BIOS, bios, bios_size);
        }
        else
        {
            memcpy(this->m_BIOS, HLE_BIOS, sizeof(HLE_BIOS));
        }

        if (!File::Exists(rom_file))
            throw runtime_error("Cannot open ROM file.");

        m_ROM = File::ReadFile(rom_file);
        m_ROMSize = File::GetFileSize(rom_file);

        // Setup Video controller
        m_Video.Init(&m_Interrupt);

        // Detect savetype
        for (int i = 0; i < m_ROMSize; i += 4)
        {
            if (memcmp(m_ROM + i, "EEPROM_V", 8) == 0)
            {
                m_SaveType = SAVE_EEPROM;
                LOG(LOG_INFO, "Found save type: EEPROM (unsupported)");
            }
            else if (memcmp(m_ROM + i, "SRAM_V", 6) == 0)
            {
                m_SaveType = SAVE_SRAM;
                m_Backup = new SRAM(save_file);
                LOG(LOG_INFO, "Found save type: SRAM");
            }
            else if (memcmp(m_ROM + i, "FLASH_V", 7) == 0 ||
                     memcmp(m_ROM + i, "FLASH512_V", 10) == 0)
            {
                m_SaveType = SAVE_FLASH64;
                m_Backup = new Flash(save_file, false);
                LOG(LOG_INFO, "Found save type: FLASH64");
            }
            else if (memcmp(m_ROM + i, "FLASH1M_V", 9) == 0)
            {
                m_SaveType = SAVE_FLASH128;
                m_Backup = new Flash(save_file, true);
                LOG(LOG_INFO, "Found save type: FLASH128");
            }
        }

        if (m_SaveType == SAVE_NONE)
        {
            m_SaveType = SAVE_SRAM;
            LOG(LOG_WARN, "Save type not determinable, default to SRAM.");
        }
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     Destructor
    ///
    ///////////////////////////////////////////////////////////
    Memory::~Memory()
    {
        delete m_ROM;
        delete m_Backup;
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     SequentialAccess
    ///
    ///////////////////////////////////////////////////////////
    int Memory::SequentialAccess(u32 offset, AccessSize size)
    {
        int page = offset >> 24;

        if (page == 2)
        {
            if (size == ACCESS_WORD) return 6;
            return 3;
        }

        if (page == 5 || page == 6)
        {
            if (size == ACCESS_WORD) return 2;
            return 1;
        }

        if (page == 8)
        {
            if (size == ACCESS_WORD)
                return 1 + 2 * WSN_TABLE[m_Waitstate.first[0]];
            return 1 + WSN_TABLE[m_Waitstate.first[0]];
        }

        if (page == 0xE)
        {
            if (size == ACCESS_WORD && m_SaveType != SAVE_SRAM) return 8;
            return 5;
        }

        return 1;
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     NonSequentialAccess
    ///
    ///////////////////////////////////////////////////////////
    int Memory::NonSequentialAccess(u32 offset, AccessSize size)
    {
        int page = offset >> 24;

        if (page == 8)
        {
            if (size == ACCESS_WORD)
                return 1 + WSS0_TABLE[m_Waitstate.second[0]] + WSN_TABLE[m_Waitstate.first[0]];
            return 1 + WSS0_TABLE[m_Waitstate.second[0]];
        }

        return SequentialAccess(offset, size);
    }

    u8 Memory::ReadBIOS(u32 address)
    {
        if (address >= 0x4000) return 0;

        return m_BIOS[address];
    }

    u8 Memory::ReadWRAM(u32 address)
    {
        return m_WRAM[address % 0x40000];
    }

    u8 Memory::ReadIRAM(u32 address)
    {
        return m_IRAM[address % 0x8000];
    }

    u8 Memory::ReadPAL(u32 address)
    {
        return m_Video.m_PAL[address % 0x400];
    }

    u8 Memory::ReadVRAM(u32 address)
    {
        address %= 0x20000;
        if (address >= 0x18000)
            address &= ~0x8000;

        return m_Video.m_VRAM[address];
    }

    u8 Memory::ReadOAM(u32 address)
    {
        return m_Video.m_OAM[address % 0x400];
    }

    u8 Memory::ReadROM(u32 address)
    {
        address &= 0x1FFFFFF;

        if (address >= m_ROMSize) return 0;

        return m_ROM[address];
    }

    u8 Memory::ReadPageE(u32 address)
    {
        if (m_Backup != nullptr && (m_SaveType == SAVE_FLASH64 ||
                m_SaveType == SAVE_FLASH128 ||
                m_SaveType == SAVE_SRAM))
            return m_Backup->ReadByte(address);
        return 0;
    }

    u8 Memory::ReadInvalid(u32 address)
    {
#ifdef DEBUG
        LOG(LOG_ERROR, "Read from invalid address: 0x%x", address);
#endif
        return 0;
    }

    void Memory::WriteWRAM(u32 address, u8 value)
    {
         m_WRAM[address % 0x40000] = value;
    }

    void Memory::WriteIRAM(u32 address, u8 value)
    {
        m_IRAM[address % 0x8000] = value;
    }

    void Memory::WritePAL(u32 address, u8 value)
    {
        address %= 0x400;
        m_Video.m_PAL[address] = value;
    }

    void Memory::WriteVRAM(u32 address, u8 value)
    {
        address %= 0x20000;
        if (address >= 0x18000)
            address &= ~0x8000;

        m_Video.m_VRAM[address] = value;
    }

    void Memory::WriteOAM(u32 address, u8 value)
    {
        address %= 0x400;
        m_Video.m_OAM[address] = value;
    }

    void Memory::WritePageE(u32 address, u8 value)
    {
        if (m_Backup != nullptr && (m_SaveType == SAVE_FLASH64 ||
                m_SaveType == SAVE_FLASH128 ||
                m_SaveType == SAVE_SRAM))
            m_Backup->WriteByte(address, value);
    }

    void Memory::WriteInvalid(u32 address, u8 value)
    {
#ifdef DEBUG
        LOG(LOG_ERROR, "Write to invalid address: 0x%x = 0x%x", address, value);
#endif
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadByte
    ///
    ///////////////////////////////////////////////////////////
    u8 Memory::ReadByte(u32 offset)
    {
        return (this->*READ_TABLE[(offset >> 24) & 15])(offset);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadHWord
    ///
    ///////////////////////////////////////////////////////////
    u16 Memory::ReadHWord(u32 offset)
    {
        register ReadFunction func = READ_TABLE[(offset >> 24) & 15];

        return (this->*func)(offset) |
               ((this->*func)(offset + 1) << 8);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadWord
    ///
    ///////////////////////////////////////////////////////////
    u32 Memory::ReadWord(u32 offset)
    {
        register ReadFunction func = READ_TABLE[(offset >> 24) & 15];

        return (this->*func)(offset) |
               ((this->*func)(offset + 1) << 8) |
               ((this->*func)(offset + 2) << 16) |
               ((this->*func)(offset + 3) << 24);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteByte
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteByte(u32 offset, u8 value)
    {
        register int page = (offset >> 24) & 15;
        register WriteFunction func = WRITE_TABLE[page];

        // Handle writes to memory that has 16-bit data bus only.
        if (page == 5 || page == 6 || page == 7)
        {
            (this->*func)(offset & ~1      , value);
            (this->*func)((offset & ~1) + 1, value);
            return;
        }

        (this->*func)(offset, value);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteHWord
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteHWord(u32 offset, u16 value)
    {
        register WriteFunction func = WRITE_TABLE[(offset >> 24) & 15];

        (this->*func)(offset    , value & 0xFF);
        (this->*func)(offset + 1, (value >> 8) & 0xFF);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteWord
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteWord(u32 offset, u32 value)
    {
        register WriteFunction func = WRITE_TABLE[(offset >> 24) & 15];

        (this->*func)(offset    , value);
        (this->*func)(offset + 1, (value >> 8) & 0xFF);
        (this->*func)(offset + 2, (value >> 16) & 0xFF);
        (this->*func)(offset + 3, (value >> 24) & 0xFF);
    }

}
