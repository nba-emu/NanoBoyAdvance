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
#include "util/log.h"
#include "util/file.h"
#include "flash.h"
#include "sram.h"
#include <stdexcept>


using namespace std;


namespace NanoboyAdvance
{
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


    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadByte
    ///
    ///////////////////////////////////////////////////////////
    u8 Memory::ReadByte(u32 offset)
    {
        int page = offset >> 24;
        u32 internal_offset = offset & 0xFFFFFF;

        switch (page)
        {
        case 0:
        case 1:
            #ifdef DEBUG
            ASSERT(internal_offset >= 0x4000, LOG_ERROR, "BIOS read: offset out of bounds");
            #endif            
            if (internal_offset >= 0x4000) 
                return 0;
            return m_BIOS[internal_offset];
        case 2:
            return m_WRAM[internal_offset % 0x40000];
        case 3:
            return m_IRAM[internal_offset % 0x8000];
        case 4:
            if ((internal_offset & 0xFFFF) == 0x800) 
                internal_offset &= 0xFFFF;
            
            switch (internal_offset) {
            case DISPCNT:
                return (m_Video.m_VideoMode) |
                       (m_Video.m_FrameSelect ? 16 : 0) |
                       (m_Video.m_Obj.hblank_access ? 32 : 0) |
                       (m_Video.m_Obj.two_dimensional ? 64 : 0) |
                       (m_Video.m_ForcedBlank ? 128 : 0);
            case DISPCNT+1:
                return (m_Video.m_BG[0].enable ? 1 : 0) |
                       (m_Video.m_BG[1].enable ? 2 : 0) |
                       (m_Video.m_BG[2].enable ? 4 : 0) |
                       (m_Video.m_BG[3].enable ? 8 : 0) |
                       (m_Video.m_Obj.enable ? 16 : 0) |
                       (m_Video.m_Win[0].enable ? 32 : 0) |
                       (m_Video.m_Win[1].enable ? 64 : 0) |
                       (m_Video.m_ObjWin.enable ? 128 : 0);
            case DISPSTAT:
                return ((m_Video.m_State == Video::RenderingPhase::VBlank) ? 1 : 0) |
                       ((m_Video.m_State == Video::RenderingPhase::HBlank) ? 2 : 0) |
                       (m_Video.m_VCountFlag ? 4 : 0) |
                       (m_Video.m_VBlankIRQ ? 8 : 0) |
                       (m_Video.m_HBlankIRQ ? 16 : 0) |
                       (m_Video.m_VCountIRQ ? 32 : 0);
            case VCOUNT:
                return m_Video.m_VCount;
            case BG0CNT:
            case BG1CNT:
            case BG2CNT:
            case BG3CNT:
            {
                int n = (internal_offset - BG0CNT) / 2;    
                return (m_Video.m_BG[n].priority) |
                       ((m_Video.m_BG[n].tile_base / 0x4000) << 2) |
                       (m_Video.m_BG[n].mosaic ? 64 : 0) |
                       (m_Video.m_BG[n].true_color ? 128 : 0) |
                       (3 << 4); // bits 4-5 are always 1
            }
            case BG0CNT+1:
            case BG1CNT+1:
            case BG2CNT+1:
            case BG3CNT+1:
            {
                int n = (internal_offset - BG0CNT - 1) / 2;
                return (m_Video.m_BG[n].map_base / 0x800) |
                       (m_Video.m_BG[n].wraparound ? 32 : 0) |
                       (m_Video.m_BG[n].size << 6);
            }
            case WININ:
                return (m_Video.m_Win[0].bg_in[0] ? 1 : 0) |
                       (m_Video.m_Win[0].bg_in[1] ? 2 : 0) |
                       (m_Video.m_Win[0].bg_in[2] ? 4 : 0) |
                       (m_Video.m_Win[0].bg_in[3] ? 8 : 0) |
                       (m_Video.m_Win[0].obj_in ? 16 : 0) |
                       (m_Video.m_Win[0].sfx_in ? 32 : 0);
            case WININ+1:
                return (m_Video.m_Win[1].bg_in[0] ? 1 : 0) |
                       (m_Video.m_Win[1].bg_in[1] ? 2 : 0) |
                       (m_Video.m_Win[1].bg_in[2] ? 4 : 0) |
                       (m_Video.m_Win[1].bg_in[3] ? 8 : 0) |
                       (m_Video.m_Win[1].obj_in ? 16 : 0) |
                       (m_Video.m_Win[1].sfx_in ? 32 : 0);
            case WINOUT:
                return (m_Video.m_WinOut.bg[0] ? 1 : 0) |
                       (m_Video.m_WinOut.bg[1] ? 2 : 0) |
                       (m_Video.m_WinOut.bg[2] ? 4 : 0) |
                       (m_Video.m_WinOut.bg[3] ? 8 : 0) |
                       (m_Video.m_WinOut.obj ? 16 : 0) |
                       (m_Video.m_WinOut.sfx ? 32 : 0);
            case WINOUT+1:
                // TODO: OBJWIN
                return 0;
            case BLDCNT:
                return (m_Video.m_SFX.bg_select[0][0] ? 1 : 0) |
                       (m_Video.m_SFX.bg_select[0][1] ? 2 : 0) |
                       (m_Video.m_SFX.bg_select[0][2] ? 4 : 0) |
                       (m_Video.m_SFX.bg_select[0][3] ? 8 : 0) |
                       (m_Video.m_SFX.obj_select[0] ? 16 : 0) |
                       (m_Video.m_SFX.bd_select[0] ? 32 : 0) |
                       (m_Video.m_SFX.effect << 6);
            case BLDCNT+1:
                return (m_Video.m_SFX.bg_select[1][0] ? 1 : 0) |
                       (m_Video.m_SFX.bg_select[1][1] ? 2 : 0) |
                       (m_Video.m_SFX.bg_select[1][2] ? 4 : 0) |
                       (m_Video.m_SFX.bg_select[1][3] ? 8 : 0) |
                       (m_Video.m_SFX.obj_select[1] ? 16 : 0) |
                       (m_Video.m_SFX.bd_select[1] ? 32 : 0);
            case SOUNDCNT_L:
                return m_Audio.m_SoundControl.master_volume_right |
                       (m_Audio.m_SoundControl.master_volume_left << 4);
            case SOUNDCNT_L+1:
                return (m_Audio.m_SoundControl.enable_right[0] ? 1 : 0) |
                       (m_Audio.m_SoundControl.enable_right[1] ? 2 : 0) |
                       (m_Audio.m_SoundControl.enable_right[2] ? 4 : 0) |
                       (m_Audio.m_SoundControl.enable_right[3] ? 8 : 0) |
                       (m_Audio.m_SoundControl.enable_left[0] ? 16 : 0) |
                       (m_Audio.m_SoundControl.enable_left[1] ? 32 : 0) |
                       (m_Audio.m_SoundControl.enable_left[2] ? 64 : 0) |
                       (m_Audio.m_SoundControl.enable_left[3] ? 128 : 0);
            case SOUNDCNT_H:
                return (m_Audio.m_SoundControl.volume) |
                       (m_Audio.m_SoundControl.dma_volume[0] ? 4 : 0) |
                       (m_Audio.m_SoundControl.dma_volume[1] ? 8 : 0);
            case SOUNDCNT_H+1:
                return (m_Audio.m_SoundControl.dma_enable_right[0] ?  1 : 0) |
                       (m_Audio.m_SoundControl.dma_enable_left[0]  ?  2 : 0) |
                       (m_Audio.m_SoundControl.dma_timer[0]        ?  4 : 0) |
                       (m_Audio.m_SoundControl.dma_enable_right[1] ? 16 : 0) |
                       (m_Audio.m_SoundControl.dma_enable_left[1]  ? 32 : 0) |
                       (m_Audio.m_SoundControl.dma_timer[1]        ? 64 : 0);
            case SOUNDBIAS:
            case SOUNDBIAS+1:
            case SOUNDBIAS+2:
            case SOUNDBIAS+3:
            {
                int n = (internal_offset - SOUNDBIAS) * 8;
                return (m_SOUNDBIAS >> n) & 0xFF;
            }
            case TM0CNT_L:
                return m_Timer[0].count & 0xFF;
            case TM0CNT_L+1:
                return m_Timer[0].count >> 8;
            case TM1CNT_L:
                return m_Timer[1].count & 0xFF;
            case TM1CNT_L+1:
                return m_Timer[1].count >> 8;
            case TM2CNT_L:
                return m_Timer[2].count & 0xFF;
            case TM2CNT_L+1:
                return m_Timer[2].count >> 8;
            case TM3CNT_L:
                return m_Timer[3].count & 0xFF;
            case TM3CNT_L+1:
                return m_Timer[3].count >> 8;
            case TM0CNT_H:
            case TM1CNT_H:
            case TM2CNT_H:
            case TM3CNT_H:
            {
                int n = (internal_offset - TM0CNT_H) / 4;
                return m_Timer[n].clock |
                       (m_Timer[n].countup ? 4 : 0) |
                       (m_Timer[n].interrupt ? 64 : 0) |
                       (m_Timer[n].enable ? 128 : 0);
            }
            case KEYINPUT: 
                return m_KeyInput & 0xFF;
            case KEYINPUT+1:
                return m_KeyInput >> 8;
            case IE:
                return m_Interrupt.ie & 0xFF;
             case IE+1:
                return m_Interrupt.ie >> 8;
            case IF:
                return m_Interrupt.if_ & 0xFF;
            case IF+1:
                return m_Interrupt.if_ >> 8;
            case WAITCNT:
                return m_Waitstate.sram |
                       (m_Waitstate.first[0] << 2) |
                       (m_Waitstate.second[0] << 4) |
                       (m_Waitstate.first[1] << 5) |
                       (m_Waitstate.second[1] << 7);
            case WAITCNT+1:
                return m_Waitstate.first[2] |
                       (m_Waitstate.second[2] << 2) |
                       (m_Waitstate.prefetch ? 64 : 0) |
                       (1 << 7);
            case IME:
                return m_Interrupt.ime & 0xFF;
            case IME+1:
                return m_Interrupt.ime >> 8;
            }
            return 0;
        case 5:
            return m_Video.m_PAL[internal_offset % 0x400];
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000)
                internal_offset -= 0x8000;
            return m_Video.m_VRAM[internal_offset];
        case 7:
            return m_Video.m_OAM[internal_offset % 0x400];
        case 8:
            if (internal_offset >= m_ROMSize) return 0;
            return m_ROM[internal_offset];
        case 9:
            internal_offset += 0x1000000;
            if (internal_offset >= m_ROMSize) return 0;
            return m_ROM[internal_offset];
        case 0xE:
            if (m_Backup != nullptr && (m_SaveType == SAVE_FLASH64 ||
                    m_SaveType == SAVE_FLASH128 ||
                    m_SaveType == SAVE_SRAM))
                return m_Backup->ReadByte(offset);
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Read from invalid/unimplemented address (0x%x)", offset);
            #endif
            break;
        }
        return 0;
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadHWord
    ///
    ///////////////////////////////////////////////////////////
    u16 Memory::ReadHWord(u32 offset)
    {
        return ReadByte(offset) | 
               (ReadByte(offset + 1) << 8);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     ReadWord
    ///
    ///////////////////////////////////////////////////////////
    u32 Memory::ReadWord(u32 offset)
    {
        return ReadByte(offset) |
               (ReadByte(offset + 1) << 8) |
               (ReadByte(offset + 2) << 16) |
               (ReadByte(offset + 3) << 24);
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteByte
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteByte(u32 offset, u8 value)
    {
        int page = offset >> 24;
        u32 internal_offset = offset & 0xFFFFFF;

        switch (page) 
        {
        case 0:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write into BIOS memory not allowed (0x%x)", offset);
            #endif           
            break;
        case 2:
            m_WRAM[internal_offset % 0x40000] = value;
            break;
        case 3:
            m_IRAM[internal_offset % 0x8000] = value;
            break;
        case 4:
        {
            // If the address it out of bounds we should exit now
            if (internal_offset >= 0x3FF && (internal_offset & 0xFFFF) != 0x800)
            {
                #ifdef DEBUG
                LOG(LOG_ERROR, "IO write: offset out of bounds (0x%x)", offset);
                #endif
                break;
            }

            // Emulate IO mirror at 04xx0800
            if ((internal_offset & 0xFFFF) == 0x800)
                internal_offset &= 0xFFFF;
            

            switch (internal_offset)
            {
            case DISPCNT:
                m_Video.m_VideoMode = value & 7;
                m_Video.m_FrameSelect = value & 16;
                m_Video.m_Obj.hblank_access = value & 32;
                m_Video.m_Obj.two_dimensional = value & 64;
                m_Video.m_ForcedBlank = value & 128;
                break;
            case DISPCNT+1:
                m_Video.m_BG[0].enable = value & 1;
                m_Video.m_BG[1].enable = value & 2;
                m_Video.m_BG[2].enable = value & 4;
                m_Video.m_BG[3].enable = value & 8;
                m_Video.m_Obj.enable = value & 16;
                m_Video.m_Win[0].enable = value & 32;
                m_Video.m_Win[1].enable = value & 64;
                m_Video.m_ObjWin.enable = value & 128;
                break;
            case DISPSTAT:
                m_Video.m_VBlankIRQ = value & 8;
                m_Video.m_HBlankIRQ = value & 16;
                m_Video.m_VCountIRQ = value & 32;
                break;
            case DISPSTAT+1:
                m_Video.m_VCountSetting = value;
                break;
            case BG0CNT:
            case BG1CNT:
            case BG2CNT:
            case BG3CNT:
            {
                int n = (internal_offset - BG0CNT) / 2;    
                m_Video.m_BG[n].priority = value & 3;
                m_Video.m_BG[n].tile_base = ((value >> 2) & 3) * 0x4000;
                m_Video.m_BG[n].mosaic = value & 64;
                m_Video.m_BG[n].true_color = value & 128;
                break;
            }
            case BG0CNT+1:
            case BG1CNT+1:
            case BG2CNT+1:
            case BG3CNT+1:
            {
                int n = (internal_offset - BG0CNT - 1) / 2;
                m_Video.m_BG[n].map_base = (value & 31) * 0x800;

                if (n == 2 || n == 3)
                    m_Video.m_BG[n].wraparound = value & 32;

                m_Video.m_BG[n].size = value >> 6;
                break;
            }
            case BG0HOFS:
            case BG1HOFS:
            case BG2HOFS:
            case BG3HOFS:
            {
                int n = (internal_offset - BG0HOFS) / 4;
                m_Video.m_BG[n].x = (m_Video.m_BG[n].x & 0x100) | value;
                break;
            }
            case BG0HOFS+1:
            case BG1HOFS+1:
            case BG2HOFS+1:
            case BG3HOFS+1:
            {
                int n = (internal_offset - BG0HOFS - 1) / 4;
                m_Video.m_BG[n].x = (m_Video.m_BG[n].x & 0xFF) | ((value & 1) << 8);
                break;
            }
            case BG0VOFS:
            case BG1VOFS:
            case BG2VOFS:
            case BG3VOFS:
            {
                int n = (internal_offset - BG0VOFS) / 4;
                m_Video.m_BG[n].y = (m_Video.m_BG[n].y & 0x100) | value;
                break;
            }
            case BG0VOFS+1:
            case BG1VOFS+1:
            case BG2VOFS+1:
            case BG3VOFS+1:
            {
                int n = (internal_offset - BG0VOFS - 1) / 4;
                m_Video.m_BG[n].y = (m_Video.m_BG[n].y & 0xFF) | ((value & 1) << 8);
                break;
            }
            case BG2X:
            case BG2X+1:
            case BG2X+2:
            case BG2X+3:
            {
                int n = (internal_offset - BG2X) * 8;
                u32 v = (m_Video.m_BG[2].x_ref & ~(0xFF << n)) | (value << n);
                m_Video.m_BG[2].x_ref = v;
                m_Video.m_BG[2].x_ref_int = Video::DecodeGBAFloat32(v);
                break;
            }
            case BG3X:
            case BG3X+1:
            case BG3X+2:
            case BG3X+3:
            {
                int n = (internal_offset - BG3X) * 8;
                u32 v = (m_Video.m_BG[3].x_ref & ~(0xFF << n)) | (value << n);
                m_Video.m_BG[3].x_ref = v;
                m_Video.m_BG[3].x_ref_int = Video::DecodeGBAFloat32(v);
                break;
            }
            case BG2Y:
            case BG2Y+1:
            case BG2Y+2:
            case BG2Y+3:
            {
                int n = (internal_offset - BG2Y) * 8;
                u32 v = (m_Video.m_BG[2].y_ref & ~(0xFF << n)) | (value << n);
                m_Video.m_BG[2].y_ref = v;
                m_Video.m_BG[2].y_ref_int = Video::DecodeGBAFloat32(v);
                break;
            }
            case BG3Y:
            case BG3Y+1:
            case BG3Y+2:
            case BG3Y+3:
            {
                int n = (internal_offset - BG3Y) * 8;
                u32 v = (m_Video.m_BG[3].y_ref & ~(0xFF << n)) | (value << n);
                m_Video.m_BG[3].y_ref = v;
                m_Video.m_BG[3].y_ref_int = Video::DecodeGBAFloat32(v);
                break;
            }
            case BG2PA:
            case BG2PA+1:
            {
                int n = (internal_offset - BG2PA) * 8;
                m_Video.m_BG[2].pa = (m_Video.m_BG[2].pa & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PA:
            case BG3PA+1:
            {
                int n = (internal_offset - BG3PA) * 8;
                m_Video.m_BG[3].pa = (m_Video.m_BG[3].pa & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG2PB:
            case BG2PB+1:
            {
                int n = (internal_offset - BG2PB) * 8;
                m_Video.m_BG[2].pb = (m_Video.m_BG[2].pb & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PB:
            case BG3PB+1:
            {
                int n = (internal_offset - BG3PB) * 8;
                m_Video.m_BG[3].pb = (m_Video.m_BG[3].pb & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG2PC:
            case BG2PC+1:
            {
                int n = (internal_offset - BG2PC) * 8;
                m_Video.m_BG[2].pc = (m_Video.m_BG[2].pc & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PC:
            case BG3PC+1:
            {
                int n = (internal_offset - BG3PC) * 8;
                m_Video.m_BG[3].pc = (m_Video.m_BG[3].pc & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG2PD:
            case BG2PD+1:
            {
                int n = (internal_offset - BG2PD) * 8;
                m_Video.m_BG[2].pd = (m_Video.m_BG[2].pd & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PD:
            case BG3PD+1:
            {
                int n = (internal_offset - BG3PD) * 8;
                m_Video.m_BG[3].pd = (m_Video.m_BG[3].pd & ~(0xFF << n)) | (value << n);
                break;
            }
            case WIN0H:
                m_Video.m_Win[0].right = value;
                break;
            case WIN0H+1:
                m_Video.m_Win[0].left = value;
                break;
            case WIN1H:
                m_Video.m_Win[1].right = value;
                break;
            case WIN1H+1:
                m_Video.m_Win[1].left = value;
                break;
            case WIN0V:
                m_Video.m_Win[0].bottom = value;
                break;
            case WIN0V+1:
                m_Video.m_Win[0].top = value;
                break;
            case WIN1V:
                m_Video.m_Win[1].bottom = value;
                break;
            case WIN1V+1:
                m_Video.m_Win[1].top = value;
                break;
            case WININ:
                m_Video.m_Win[0].bg_in[0] = value & 1;
                m_Video.m_Win[0].bg_in[1] = value & 2;
                m_Video.m_Win[0].bg_in[2] = value & 4;
                m_Video.m_Win[0].bg_in[3] = value & 8;
                m_Video.m_Win[0].obj_in = value & 16;
                m_Video.m_Win[0].sfx_in = value & 32;
                break;
            case WININ+1:
                m_Video.m_Win[1].bg_in[0] = value & 1;
                m_Video.m_Win[1].bg_in[1] = value & 2;
                m_Video.m_Win[1].bg_in[2] = value & 4;
                m_Video.m_Win[1].bg_in[3] = value & 8;
                m_Video.m_Win[1].obj_in = value & 16;
                m_Video.m_Win[1].sfx_in = value & 32;
                break;
            case WINOUT:
                m_Video.m_WinOut.bg[0] = value & 1;
                m_Video.m_WinOut.bg[1] = value & 2;
                m_Video.m_WinOut.bg[2] = value & 4;
                m_Video.m_WinOut.bg[3] = value & 8;
                m_Video.m_WinOut.obj = value & 16;
                m_Video.m_WinOut.sfx = value & 32;
                break;
            case WINOUT+1:
                // TODO: OBJWIN
                break;
            case BLDCNT:
                m_Video.m_SFX.bg_select[0][0] = value & 1;
                m_Video.m_SFX.bg_select[0][1] = value & 2;
                m_Video.m_SFX.bg_select[0][2] = value & 4;
                m_Video.m_SFX.bg_select[0][3] = value & 8;
                m_Video.m_SFX.obj_select[0] = value & 16;
                m_Video.m_SFX.bd_select[0] = value & 32;
                m_Video.m_SFX.effect = static_cast<SpecialEffect::Effect>(value >> 6);
                break;
            case BLDCNT+1:
                m_Video.m_SFX.bg_select[1][0] = value & 1;
                m_Video.m_SFX.bg_select[1][1] = value & 2;
                m_Video.m_SFX.bg_select[1][2] = value & 4;
                m_Video.m_SFX.bg_select[1][3] = value & 8;
                m_Video.m_SFX.obj_select[1] = value & 16;
                m_Video.m_SFX.bd_select[1] = value & 32;
                break;
            case BLDALPHA:
                m_Video.m_SFX.eva = value & 0x1F;
                break;
            case BLDALPHA+1:
                m_Video.m_SFX.evb = value & 0x1F;
                break;
            case BLDY:
                m_Video.m_SFX.evy = value & 0x1F;
                break;
            case SOUNDCNT_L:
                m_Audio.m_SoundControl.master_volume_right = value & 7;
                m_Audio.m_SoundControl.master_volume_left = (value >> 4) & 7;
                break;
            case SOUNDCNT_L+1:
                m_Audio.m_SoundControl.enable_right[0] = value & 1;
                m_Audio.m_SoundControl.enable_right[1] = value & 2;
                m_Audio.m_SoundControl.enable_right[2] = value & 4;
                m_Audio.m_SoundControl.enable_right[3] = value & 8;
                m_Audio.m_SoundControl.enable_left[0] = value & 16;
                m_Audio.m_SoundControl.enable_left[1] = value & 32;
                m_Audio.m_SoundControl.enable_left[2] = value & 64;
                m_Audio.m_SoundControl.enable_left[3] = value & 128;
                break;
            case SOUNDCNT_H:
                m_Audio.m_SoundControl.volume = value & 3;
                m_Audio.m_SoundControl.dma_volume[0] = (value >> 2) & 1;
                m_Audio.m_SoundControl.dma_volume[1] = (value >> 3) & 1;
                break;
            case SOUNDCNT_H+1:
                m_Audio.m_SoundControl.dma_enable_right[0] = value & 1;
                m_Audio.m_SoundControl.dma_enable_left[0] = value & 2;
                m_Audio.m_SoundControl.dma_timer[0] = value & 4;
                m_Audio.m_SoundControl.dma_enable_right[1] = value & 16;
                m_Audio.m_SoundControl.dma_enable_left[1] = value & 32;
                m_Audio.m_SoundControl.dma_timer[1] = value & 64;

                if (value & 8) m_Audio.m_FIFO[0].Reset();
                if (value & 128) m_Audio.m_FIFO[1].Reset();
                break;
            case SOUNDBIAS:
            case SOUNDBIAS+1:
            case SOUNDBIAS+2:
            case SOUNDBIAS+3:
            {
                int n = (internal_offset - SOUNDBIAS) * 8;
                m_SOUNDBIAS = (m_SOUNDBIAS & ~(0xFF << n)) | (value << n);
                break;
            }
            case FIFO_A_L:
            case FIFO_A_L+1:
            case FIFO_A_H:
            case FIFO_A_H+1:
                m_Audio.m_FIFO[0].Enqueue(value);
                break;
            case FIFO_B_L:
            case FIFO_B_L+1:
            case FIFO_B_H:
            case FIFO_B_H+1:
                m_Audio.m_FIFO[1].Enqueue(value);
                break;
            case DMA0SAD:
            case DMA0SAD+1:
            case DMA0SAD+2:
            case DMA0SAD+3:
            {
                int n = (internal_offset - DMA0SAD) * 8;
                m_DMA[0].source = (m_DMA[0].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1SAD:
            case DMA1SAD+1:
            case DMA1SAD+2:
            case DMA1SAD+3:
            {
                int n = (internal_offset - DMA1SAD) * 8;
                m_DMA[1].source = (m_DMA[1].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2SAD:
            case DMA2SAD+1:
            case DMA2SAD+2:
            case DMA2SAD+3:
            {
                int n = (internal_offset - DMA2SAD) * 8;
                m_DMA[2].source = (m_DMA[2].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3SAD:
            case DMA3SAD+1:
            case DMA3SAD+2:
            case DMA3SAD+3:
            {
                int n = (internal_offset - DMA3SAD) * 8;
                m_DMA[3].source = (m_DMA[3].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0DAD:
            case DMA0DAD+1:
            case DMA0DAD+2:
            case DMA0DAD+3:
            {
                int n = (internal_offset - DMA0DAD) * 8;
                m_DMA[0].dest = (m_DMA[0].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1DAD:
            case DMA1DAD+1:
            case DMA1DAD+2:
            case DMA1DAD+3:
            {
                int n = (internal_offset - DMA1DAD) * 8;
                m_DMA[1].dest = (m_DMA[1].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2DAD:
            case DMA2DAD+1:
            case DMA2DAD+2:
            case DMA2DAD+3:
            {
                int n = (internal_offset - DMA2DAD) * 8;
                m_DMA[2].dest = (m_DMA[2].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3DAD:
            case DMA3DAD+1:
            case DMA3DAD+2:
            case DMA3DAD+3:
            {
                int n = (internal_offset - DMA3DAD) * 8;
                m_DMA[3].dest = (m_DMA[3].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0CNT_L:
                m_DMA[0].count = (m_DMA[0].count & 0xFF00) | value;
                break;
            case DMA0CNT_L+1:
                m_DMA[0].count = (m_DMA[0].count & 0x00FF) | (value << 8);
                break;
            case DMA1CNT_L:
                m_DMA[1].count = (m_DMA[1].count & 0xFF00) | value;
                break;
            case DMA1CNT_L+1:
                m_DMA[1].count = (m_DMA[1].count & 0x00FF) | (value << 8);
                break;
            case DMA2CNT_L:
                m_DMA[2].count = (m_DMA[2].count & 0xFF00) | value;
                break;
            case DMA2CNT_L+1:
                m_DMA[2].count = (m_DMA[2].count & 0x00FF) | (value << 8);
                break;
            case DMA3CNT_L:
                m_DMA[3].count = (m_DMA[3].count & 0xFF00) | value;
                break;
            case DMA3CNT_L+1:
                m_DMA[3].count = (m_DMA[3].count & 0x00FF) | (value << 8);
                break;
            case DMA0CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(m_DMA[0].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                m_DMA[0].source_control = static_cast<AddressControl>(source_control);
                m_DMA[0].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA0CNT_H+1:
            {
                int source_control = static_cast<int>(m_DMA[0].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                m_DMA[0].source_control = static_cast<AddressControl>(source_control);
                m_DMA[0].repeat = value & 2;
                m_DMA[0].size = static_cast<TransferSize>((value >> 2) & 1);
                m_DMA[0].gamepack_drq = value & 8;
                m_DMA[0].start_time = static_cast<StartTime>((value >> 4) & 3);
                m_DMA[0].interrupt = value & 64;
                m_DMA[0].enable = value & 128;
            
                if (m_DMA[0].enable) 
                {
                    m_DMA[0].source_int = m_DMA[0].source & DMA_SOURCE_MASK[0];
                    m_DMA[0].dest_int = m_DMA[0].dest & DMA_DEST_MASK[0];
                    m_DMA[0].count_int = m_DMA[0].count & DMA_COUNT_MASK[0];

                    if (m_DMA[0].count_int == 0) 
                        m_DMA[0].count_int = DMA_COUNT_MASK[0] + 1;
                }
                break;
            }
            case DMA1CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(m_DMA[1].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                m_DMA[1].source_control = static_cast<AddressControl>(source_control);
                m_DMA[1].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA1CNT_H+1:
            {
                int source_control = static_cast<int>(m_DMA[1].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                m_DMA[1].source_control = static_cast<AddressControl>(source_control);
                m_DMA[1].repeat = value & 2;
                m_DMA[1].size = static_cast<TransferSize>((value >> 2) & 1);
                m_DMA[1].gamepack_drq = value & 8;
                m_DMA[1].start_time = static_cast<StartTime>((value >> 4) & 3);
                m_DMA[1].interrupt = value & 64;
                m_DMA[1].enable = value & 128;
                
                if (m_DMA[1].enable) 
                {
                    m_DMA[1].source_int = m_DMA[1].source & DMA_SOURCE_MASK[1];
                    m_DMA[1].dest_int = m_DMA[1].dest & DMA_DEST_MASK[1];
                    m_DMA[1].count_int = m_DMA[1].count & DMA_COUNT_MASK[1];

                    if (m_DMA[1].count_int == 0) 
                        m_DMA[1].count_int = DMA_COUNT_MASK[1] + 1;
                }
                break;
            }
            case DMA2CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(m_DMA[2].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                m_DMA[2].source_control = static_cast<AddressControl>(source_control);
                m_DMA[2].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA2CNT_H+1:
            {
                int source_control = static_cast<int>(m_DMA[2].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                m_DMA[2].source_control = static_cast<AddressControl>(source_control);
                m_DMA[2].repeat = value & 2;
                m_DMA[2].size = static_cast<TransferSize>((value >> 2) & 1);
                m_DMA[2].gamepack_drq = value & 8;
                m_DMA[2].start_time = static_cast<StartTime>((value >> 4) & 3);
                m_DMA[2].interrupt = value & 64;
                m_DMA[2].enable = value & 128;
                
                if (m_DMA[2].enable) 
                {
                    m_DMA[2].source_int = m_DMA[2].source & DMA_SOURCE_MASK[2];
                    m_DMA[2].dest_int = m_DMA[2].dest & DMA_DEST_MASK[2];
                    m_DMA[2].count_int = m_DMA[2].count & DMA_COUNT_MASK[2];

                    if (m_DMA[2].count_int == 0) 
                        m_DMA[2].count_int = DMA_COUNT_MASK[2] + 1;
                }
                break;
            }
            case DMA3CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(m_DMA[3].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                m_DMA[3].source_control = static_cast<AddressControl>(source_control);
                m_DMA[3].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA3CNT_H+1:
            {
                int source_control = static_cast<int>(m_DMA[3].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                m_DMA[3].source_control = static_cast<AddressControl>(source_control);
                m_DMA[3].repeat = value & 2;
                m_DMA[3].size = static_cast<TransferSize>((value >> 2) & 1);
                m_DMA[3].gamepack_drq = value & 8;
                m_DMA[3].start_time = static_cast<StartTime>((value >> 4) & 3);
                m_DMA[3].interrupt = value & 64;
                m_DMA[3].enable = value & 128;
                
                if (m_DMA[3].enable) 
                {
                    m_DMA[3].source_int = m_DMA[3].source & DMA_SOURCE_MASK[3];
                    m_DMA[3].dest_int = m_DMA[3].dest & DMA_DEST_MASK[3];
                    m_DMA[3].count_int = m_DMA[3].count & DMA_COUNT_MASK[3];

                    if (m_DMA[3].count_int == 0) 
                        m_DMA[3].count_int = DMA_COUNT_MASK[3] + 1;
                }
                break;
            }
            case TM0CNT_L:
                m_Timer[0].reload = (m_Timer[0].reload & 0xFF00) | value;
                break;
            case TM0CNT_L+1:
                m_Timer[0].reload = (m_Timer[0].reload & 0x00FF) | (value << 8);
                break;
            case TM1CNT_L:
                m_Timer[1].reload = (m_Timer[1].reload & 0xFF00) | value;
                break;
            case TM1CNT_L+1:
                m_Timer[1].reload = (m_Timer[1].reload & 0x00FF) | (value << 8);
                break;
            case TM2CNT_L:
                m_Timer[2].reload = (m_Timer[2].reload & 0xFF00) | value;
                break;
            case TM2CNT_L+1:
                m_Timer[2].reload = (m_Timer[2].reload & 0x00FF) | (value << 8);
                break;
            case TM3CNT_L:
                m_Timer[3].reload = (m_Timer[3].reload & 0xFF00) | value;
                break;
            case TM3CNT_L+1:
                m_Timer[3].reload = (m_Timer[3].reload & 0x00FF) | (value << 8);
                break;
             case TM0CNT_H:
             case TM1CNT_H:
             case TM2CNT_H:
             case TM3CNT_H:
             {
                int n = (internal_offset - TM0CNT_H) / 4;
                m_Timer[n].clock = value & 3;
                m_Timer[n].countup = value & 4;
                m_Timer[n].interrupt = value & 64;
                m_Timer[n].enable = value & 128;
                break;
             }
            case IE:
                m_Interrupt.ie = (m_Interrupt.ie & 0xFF00) | value;
                break;
            case IE+1:
                m_Interrupt.ie = (m_Interrupt.ie & 0x00FF) | (value << 8);
                break;
            case IF:
                m_Interrupt.if_ &= ~value;
                break;
            case IF+1:
                m_Interrupt.if_ &= ~(value << 8);
                break;
            case WAITCNT:
                m_Waitstate.sram = value & 3;
                m_Waitstate.first[0] = (value >> 2) & 3;
                m_Waitstate.second[0] = (value >> 4) & 1;
                m_Waitstate.first[1] = (value >> 5) & 3;
                m_Waitstate.second[1] = value >> 7;
                break;
            case WAITCNT+1:
                m_Waitstate.first[2] = value & 3;
                m_Waitstate.second[2] = (value >> 2) & 1;
                m_Waitstate.prefetch = value & 64;
                break;
            case IME:
                m_Interrupt.ime = (m_Interrupt.ime & 0xFF00) | value;
                break;
            case IME+1:
                m_Interrupt.ime = (m_Interrupt.ime & 0x00FF) | (value << 8);
                break;
            case HALTCNT:
                m_HaltState = (value & 0x80) ? HALTSTATE_STOP : HALTSTATE_HALT;
                break;
            }
            break;
        }
        case 5:
        case 6:
        case 7:
            WriteHWord(offset & ~1, (value << 8) | value);
            break;
        case 8:
        case 9:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write into ROM memory not allowed (0x%x)", offset);
            #endif                       
            break;
        case 0xE:
            if (m_Backup != nullptr && (m_SaveType == SAVE_FLASH64 ||
                    m_SaveType == SAVE_FLASH128 ||
                    m_SaveType == SAVE_SRAM))
                m_Backup->WriteByte(offset, value); 
            break;
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write to invalid/unimplemented address (0x%x)", offset);
            #endif           
            break;
        }
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteHWord
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteHWord(u32 offset, u16 value)
    {
        int page = (offset >> 24) & 0xF;
        u32 internal_offset = offset & 0xFFFFFF;

        switch (page)
        {
        case 5:
            m_Video.m_PAL[internal_offset % 0x400] = value & 0xFF;
            m_Video.m_PAL[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            return;
        case 6:
            internal_offset %= 0x20000;

            if (internal_offset >= 0x18000)
                internal_offset -= 0x8000;
            
            m_Video.m_VRAM[internal_offset] = value & 0xFF;
            m_Video.m_VRAM[internal_offset + 1] = (value >> 8) & 0xFF;
            return;
        case 7:
            m_Video.m_OAM[internal_offset % 0x400] = value & 0xFF;
            m_Video.m_OAM[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            return;
        default:
            WriteByte(offset, value & 0xFF);
            WriteByte(offset + 1, (value >> 8) & 0xFF);
            return;
        }
    }

    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     WriteWord
    ///
    ///////////////////////////////////////////////////////////
    void Memory::WriteWord(u32 offset, u32 value)
    {
        WriteHWord(offset, value & 0xFFFF);
        WriteHWord(offset + 2, (value >> 16) & 0xFFFF);
        
        // Handle special timer behaviour
        if (value & (1 << 23))
        {
            switch (offset) 
            {
            case TM0CNT_L|0x04000000:
                m_Timer[0].count = m_Timer[0].reload;
                break;
            case TM1CNT_L|0x04000000:
                m_Timer[1].count = m_Timer[1].reload;
                break;
            case TM2CNT_L|0x04000000:
                m_Timer[2].count = m_Timer[2].reload;
                break;
            case TM3CNT_L|0x04000000:
                m_Timer[3].count = m_Timer[3].reload;
                break;
            }
        }
    }

}
