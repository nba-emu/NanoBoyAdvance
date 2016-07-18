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

#include "memory.h"
#include "util/log.h"
#include "util/file.h"
#include "flash.h"
#include "sram.h"

using namespace std;

namespace NanoboyAdvance
{
    const int GBAMemory::dma_count_mask[4] = {0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF};
    const int GBAMemory::dma_dest_mask[4] = {0x7FFFFFF, 0x7FFFFFF, 0x7FFFFFF, 0xFFFFFFF};
    const int GBAMemory::dma_source_mask[4] = {0x7FFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF};
    const int GBAMemory::tmr_cycles[4] = {1, 64, 256, 1024};
    
    const int GBAMemory::wsn_table[4] = {4, 3, 2, 8};
    const int GBAMemory::wss0_table[2] = {2, 1};
    const int GBAMemory::wss1_table[2] = {4, 1};
    const int GBAMemory::wss2_table[2] = {8, 1};
    
    const u8 GBAMemory::hle_bios[0x40] = {
        0x06, 0x00, 0x00, 0xEA, 0x00, 0x00, 0xA0, 0xE1,
        0x00, 0x00, 0xA0, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
        0x00, 0x00, 0xA0, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
        0x01, 0x00, 0x00, 0xEA, 0x00, 0x00, 0xA0, 0xE1,
        0x02, 0xF3, 0xA0, 0xE3, 0x0F, 0x50, 0x2D, 0xE9,
        0x01, 0x03, 0xA0, 0xE3, 0x00, 0xE0, 0x8F, 0xE2,
        0x04, 0xF0, 0x10, 0xE5, 0x0F, 0x50, 0xBD, 0xE8,
        0x04, 0xF0, 0x5E, 0xE2, 0x00, 0x00, 0xA0, 0xE1   
    };

    GBAMemory::GBAMemory(string rom_file, string save_file) : GBAMemory(rom_file, save_file, nullptr, 0)
    {}

    GBAMemory::GBAMemory(string rom_file, string save_file, u8* bios, size_t bios_size)
    {
        // Init memory buffers
        memset(this->bios, 0, 0x4000);
        memset(wram, 0, 0x40000);
        memset(iram, 0, 0x8000);

        // Load BIOS memory
        if (bios != nullptr)
        {
            if (bios_size > 0x4000)
                throw new runtime_error("BIOS file is too big.");
            memcpy(this->bios, bios, bios_size);
        }
        else
        {
            memcpy(this->bios, hle_bios, sizeof(hle_bios));
        }

        if (!File::Exists(rom_file))
            throw new runtime_error("Cannot open ROM.");

        rom = File::ReadFile(rom_file);
        rom_size = File::GetFileSize(rom_file);

        // Setup Video and Interrupt hardware
        interrupt = new GBAInterrupt();
        interrupt->ie = 0;
        interrupt->if_ = 0;
        interrupt->ime = 0;
        video = new GBAVideo(interrupt);
        
        // Detect savetype
        for (int i = 0; i < rom_size; i += 4)
        {
            if (memcmp(rom + i, "EEPROM_V", 8) == 0)
            {
                save_type = SaveType::EEPROM;
                LOG(LOG_INFO, "Found save type: EEPROM (unsupported)");
            }
            else if (memcmp(rom + i, "SRAM_V", 6) == 0)
            {
                save_type = SaveType::SRAM;
                backup = new SRAM(save_file);
                LOG(LOG_INFO, "Found save type: SRAM");
            }
            else if (memcmp(rom + i, "FLASH_V", 7) == 0 ||
                     memcmp(rom + i, "FLASH512_V", 10) == 0)
            {
                save_type = SaveType::FLASH64;
                backup = new GBAFlash(save_file, false);
                LOG(LOG_INFO, "Found save type: FLASH64");
            }
            else if (memcmp(rom + i, "FLASH1M_V", 9) == 0)
            {
                save_type = SaveType::FLASH128;
                backup = new GBAFlash(save_file, true);
                LOG(LOG_INFO, "Found save type: FLASH128");
            }
        }

        if (save_type == SaveType::NONE)
        {
            save_type = SaveType::SRAM;
            LOG(LOG_WARN, "Save type not determinable, default to SRAM.");
        }
    }

    GBAMemory::~GBAMemory()
    {
        delete backup;
        delete video;
        delete interrupt;
    }
    
    void GBAMemory::RunTimer()
    {
        bool overflow = false;
        
        for (int i = 0; i < 4; i++)
        {
            if (!timer[i].enable) continue;

            if ((timer[i].countup && overflow) || 
                (!timer[i].countup && ++timer[i].ticks >= tmr_cycles[timer[i].clock]))
            {
                timer[i].ticks = 0;
            
                if (timer[i].count != 0xFFFF)
                {
                    timer[i].count++;
                    overflow = false;
                }
                else 
                {
                    if (timer[i].interrupt) interrupt->if_ |= 8 << i;
                    timer[i].count = timer[i].reload;
                    overflow = true;
                }
            }
        }
    }

    void GBAMemory::RunDMA()
    {
        // TODO: FIFO A/B and Video Capture
        for (int i = 0; i < 4; i++)
        {
            bool start = false;

            if (!dma[i].enable) continue;
                
            switch (dma[i].start_time) 
            {
            case StartTime::Immediate:
                start = true;
                break;
            case StartTime::VBlank:
                if (video->vblank_dma)
                {
                    start = true;
                    video->vblank_dma = false;
                }
                break;
            case StartTime::HBlank:
                if (video->hblank_dma)
                {
                    start = true;
                    video->hblank_dma = false;
                }
                break;
            case StartTime::Special:
                #ifdef DEBUG
                ASSERT(i == 3, LOG_ERROR, "DMA: Video Capture Mode not supported.");
                #endif
                break;
            }
                
            if (start)
            {
                AddressControl dest_control = dma[i].dest_control;
                AddressControl source_control = dma[i].source_control;
                bool transfer_words = dma[i].size == TransferSize::Word;

                #if DEBUG
                u32 value = ReadHWord(dma[i].source);
                LOG(LOG_INFO, "DMA%d: s=%x d=%x c=%x count=%x l=%d v=%x", i, dma[i].source_int,
                               dma[i].dest_int, 0, dma[i].count_int, video->vcount, value);
                ASSERT(dma[i].gamepack_drq, LOG_ERROR, "Game Pak DRQ not supported.");
                #endif
                                        
                // Run as long as there is data to transfer
                while (dma[i].count_int != 0)
                {
                    // Transfer either Word or HWord
                    if (transfer_words) 
                        WriteWord(dma[i].dest_int & ~3, ReadWord(dma[i].source_int & ~3));
                    else 
                        WriteHWord(dma[i].dest_int & ~1, ReadHWord(dma[i].source_int & ~1));
                        
                    // Update destination address
                    if (dest_control == AddressControl::Increment || dest_control == AddressControl::Reload)
                        dma[i].dest_int += transfer_words ? 4 : 2;
                    else if (dest_control == AddressControl::Decrement)
                        dma[i].dest_int -= transfer_words ? 4 : 2;
                        
                    // Update source address
                    if (source_control == AddressControl::Increment || source_control == AddressControl::Reload)
                        dma[i].source_int += transfer_words ? 4 : 2;
                    else if (source_control == AddressControl::Decrement)
                        dma[i].source_int -= transfer_words ? 4 : 2;
                        
                    // Update count
                    dma[i].count_int--;
                }
                
                // Reschedule the DMA as specified or disable it
                if (dma[i].repeat)
                {
                    dma[i].count_int = dma[i].count & dma_count_mask[i];
                    if (dma[i].count_int == 0)
                        dma[i].count_int = dma_count_mask[i] + 1;
                    if (dest_control == AddressControl::Reload)
                        dma[i].dest_int  = dma[i].dest & dma_dest_mask[i];
                } 
                else 
                {
                    dma[i].enable = false;
                }
                    
                // Raise DMA interrupt if enabled
                if (dma[i].interrupt) interrupt->if_ |= 256 << i;
            }
        }
    }

    int GBAMemory::SequentialAccess(u32 offset, AccessSize size) 
    {
        int page = offset >> 24;

        if (page == 2) 
        {
            if (size == AccessSize::Word) return 6;
            return 3;
        }

        if (page == 5 || page == 6)
        {
            if (size == AccessSize::Word) return 2;
            return 1;
        }

        if (page == 8)
        {
            if (size == AccessSize::Word) 
                return 1 + 2 * wsn_table[waitstate.first[0]];
            return 1 + wsn_table[waitstate.first[0]];
        }

        if (page == 0xE)
        {
            if (size == AccessSize::Word && save_type != SaveType::SRAM) return 8;
            return 5;
        }

        return 1;
    }

    int GBAMemory::NonSequentialAccess(u32 offset, AccessSize size)
    {
        int page = offset >> 24;

        if (page == 8) 
        {
            if (size == AccessSize::Word)
                return 1 + wss0_table[waitstate.second[0]] + wsn_table[waitstate.first[0]];
            return 1 + wss0_table[waitstate.second[0]];
        }

        return SequentialAccess(offset, size);
    }

    u8 GBAMemory::ReadByte(u32 offset)
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
            return bios[internal_offset];
        case 2:
            return wram[internal_offset % 0x40000];
        case 3:
            return iram[internal_offset % 0x8000];
        case 4:
            if ((internal_offset & 0xFFFF) == 0x800) 
                internal_offset &= 0xFFFF;
            
            switch (internal_offset) {
            case DISPCNT:
                return (video->video_mode) |
                       (video->frame_select ? 16 : 0) | 
                       (video->oam_access ? 32 : 0) |
                       (video->oam_mapping ? 64 : 0) |
                       (video->forced_blank ? 128 : 0);
            case DISPCNT+1:
                return (video->bg_enable[0] ? 1 : 0) |
                       (video->bg_enable[1] ? 2 : 0) |
                       (video->bg_enable[2] ? 4 : 0) |
                       (video->bg_enable[3] ? 8 : 0) |
                       (video->obj_enable ? 16 : 0) |
                       (video->win_enable[0] ? 32 : 0) |
                       (video->win_enable[1] ? 64 : 0) |
                       (video->obj_win_enable ? 128 : 0);
            case DISPSTAT:
                return ((video->state == GBAVideo::GBAVideoState::VBlank) ? 1 : 0) |
                       ((video->state == GBAVideo::GBAVideoState::HBlank) ? 2 : 0) |
                       (video->vcount_flag ? 4 : 0) |
                       (video->vblank_irq ? 8 : 0) |
                       (video->hblank_irq ? 16 : 0) |
                       (video->vcount_irq ? 32 : 0);
            case VCOUNT:
                return video->vcount;
            case BG0CNT:
            case BG1CNT:
            case BG2CNT:
            case BG3CNT:
            {
                int n = (internal_offset - BG0CNT) / 2;    
                return (video->bg_priority[n]) | 
                       ((video->bg_tile_base[n] / 0x4000) << 2) |
                       (video->bg_mosaic[n] ? 64 : 0) |
                       (video->bg_pal_256[n] ? 128 : 0) |
                       (3 << 4); // bits 4-5 are always 1
            }
            case BG0CNT+1:
            case BG1CNT+1:
            case BG2CNT+1:
            case BG3CNT+1:
            {
                int n = (internal_offset - BG0CNT - 1) / 2;
                return (video->bg_map_base[n] / 0x800) |
                       (video->bg_wraparound[n] ? 32 : 0) |
                       (video->bg_size[n] << 6);
            }
            case WININ:
                return (video->bg_winin[0][0] ? 1 : 0) |
                       (video->bg_winin[0][1] ? 2 : 0) |
                       (video->bg_winin[0][2] ? 4 : 0) |
                       (video->bg_winin[0][3] ? 8 : 0) |
                       (video->obj_winin[0] ? 16 : 0) |
                       (video->sfx_winin[0] ? 32 : 0);
            case WININ+1:
                return (video->bg_winin[1][0] ? 1 : 0) |
                       (video->bg_winin[1][1] ? 2 : 0) |
                       (video->bg_winin[1][2] ? 4 : 0) |
                       (video->bg_winin[1][3] ? 8 : 0) |
                       (video->obj_winin[1] ? 16 : 0) |
                       (video->sfx_winin[1] ? 32 : 0);
            case WINOUT:
                return (video->bg_winout[0] ? 1 : 0) |
                       (video->bg_winout[1] ? 2 : 0) |
                       (video->bg_winout[2] ? 4 : 0) |
                       (video->bg_winout[3] ? 8 : 0) |
                       (video->obj_winout ? 16 : 0) |
                       (video->sfx_winout ? 32 : 0);
            case WINOUT+1:
                // TODO: OBJWIN
                return 0;
            case TM0CNT_L:
                return timer[0].count & 0xFF;
            case TM0CNT_L+1:
                return timer[0].count >> 8;
            case TM1CNT_L:
                return timer[1].count & 0xFF;
            case TM1CNT_L+1:
                return timer[1].count >> 8;
            case TM2CNT_L:
                return timer[2].count & 0xFF;
            case TM2CNT_L+1:
                return timer[2].count >> 8;
            case TM3CNT_L:
                return timer[3].count & 0xFF;
            case TM3CNT_L+1:
                return timer[3].count >> 8;
            case TM0CNT_H:
            case TM1CNT_H:
            case TM2CNT_H:
            case TM3CNT_H:
            {
                int n = (internal_offset - TM0CNT_H) / 4;
                return timer[n].clock |
                       (timer[n].countup ? 4 : 0) |
                       (timer[n].interrupt ? 64 : 0) |
                       (timer[n].enable ? 128 : 0);
            }
            case KEYINPUT: 
                return keyinput & 0xFF;
            case KEYINPUT+1:
                return keyinput >> 8;
            case IE:
                return interrupt->ie & 0xFF;
             case IE+1:
                return interrupt->ie >> 8;
            case IF:
                return interrupt->if_ & 0xFF;
            case IF+1:
                return interrupt->if_ >> 8;
            case WAITCNT:
                return waitstate.sram |
                       (waitstate.first[0] << 2) |
                       (waitstate.second[0] << 4) |
                       (waitstate.first[1] << 5) |
                       (waitstate.second[1] << 7);
            case WAITCNT+1:
                return waitstate.first[2] |
                       (waitstate.second[2] << 2) |
                       (waitstate.prefetch ? 64 : 0) |
                       (1 << 7);
            case IME:
                return interrupt->ime & 0xFF;
            case IME+1:
                return interrupt->ime >> 8;
            }
            return 0;
        case 5:
            return video->pal[internal_offset % 0x400];
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000)
                internal_offset -= 0x8000;
            return video->vram[internal_offset];
        case 7:
            return video->obj[internal_offset % 0x400];
        case 8:
            if (internal_offset >= rom_size) return 0;
            return rom[internal_offset];
        case 9:
            internal_offset += 0x1000000;
            if (internal_offset >= rom_size) return 0;
            return rom[internal_offset];
        case 0xE:
            return backup->ReadByte(offset);
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Read from invalid/unimplemented address (0x%x)", offset);
            #endif
            break;
        }
        return 0;
    }

    u16 GBAMemory::ReadHWord(u32 offset)
    {
        return ReadByte(offset) | 
               (ReadByte(offset + 1) << 8);
    }

    u32 GBAMemory::ReadWord(u32 offset)
    {
        return ReadByte(offset) |
               (ReadByte(offset + 1) << 8) |
               (ReadByte(offset + 2) << 16) |
               (ReadByte(offset + 3) << 24);
    }

    void GBAMemory::WriteByte(u32 offset, u8 value)
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
            wram[internal_offset % 0x40000] = value;
            break;
        case 3:
            iram[internal_offset % 0x8000] = value;
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
                video->video_mode = value & 7;
                video->frame_select = value & 16;
                video->oam_access = value & 32;
                video->oam_mapping = value & 64;
                video->forced_blank = value & 128;
                break;
            case DISPCNT+1:
                video->bg_enable[0] = value & 1;
                video->bg_enable[1] = value & 2;
                video->bg_enable[2] = value & 4;
                video->bg_enable[3] = value & 8;
                video->obj_enable = value & 16;
                video->win_enable[0] = value & 32;
                video->win_enable[1] = value & 64;
                video->obj_win_enable = value & 128;
                break;
            case DISPSTAT:
                video->vblank_irq = value & 8;
                video->hblank_irq = value & 16;
                video->vcount_irq = value & 32;
                break;
            case DISPSTAT+1:
                video->vcount_setting = value;
                break;
            case BG0CNT:
            case BG1CNT:
            case BG2CNT:
            case BG3CNT:
            {
                int n = (internal_offset - BG0CNT) / 2;    
                video->bg_priority[n] = value & 3; 
                video->bg_tile_base[n] = ((value >> 2) & 3) * 0x4000;
                video->bg_mosaic[n] = value & 64;
                video->bg_pal_256[n] = value & 128;
                break;
            }
            case BG0CNT+1:
            case BG1CNT+1:
            case BG2CNT+1:
            case BG3CNT+1:
            {
                int n = (internal_offset - BG0CNT - 1) / 2;
                video->bg_map_base[n] = (value & 31) * 0x800;

                if (n == 2 || n == 3)
                    video->bg_wraparound[n] = value & 32;

                video->bg_size[n] = value >> 6;
                break;
            }
            case BG0HOFS:
            case BG1HOFS:
            case BG2HOFS:
            case BG3HOFS:
            {
                int n = (internal_offset - BG0HOFS) / 4;
                video->bg_hofs[n] = (video->bg_hofs[n] & 0x100) | value;
                break;
            }
            case BG0HOFS+1:
            case BG1HOFS+1:
            case BG2HOFS+1:
            case BG3HOFS+1:
            {
                int n = (internal_offset - BG0HOFS - 1) / 4;
                video->bg_hofs[n] = (video->bg_hofs[n] & 0xFF) | ((value & 1) << 8);
                break;
            }
            case BG0VOFS:
            case BG1VOFS:
            case BG2VOFS:
            case BG3VOFS:
            {
                int n = (internal_offset - BG0VOFS) / 4;
                video->bg_vofs[n] = (video->bg_vofs[n] & 0x100) | value;
                break;
            }
            case BG0VOFS+1:
            case BG1VOFS+1:
            case BG2VOFS+1:
            case BG3VOFS+1:
            {
                int n = (internal_offset - BG0VOFS - 1) / 4;
                video->bg_vofs[n] = (video->bg_vofs[n] & 0xFF) | ((value & 1) << 8);
                break;
            }
            case BG2X:
            case BG2X+1:
            case BG2X+2:
            case BG2X+3:
            {
                int n = (internal_offset - BG2X) * 8;
                u32 v = (video->bg_x[2] & ~(0xFF << n)) | (value << n);
                video->bg_x[2] = v;
                video->bg_x_int[2] = GBAVideo::DecodeGBAFloat32(v);
                break;
            }
            case BG3X:
            case BG3X+1:
            case BG3X+2:
            case BG3X+3:
            {
                int n = (internal_offset - BG3X) * 8;
                u32 v = (video->bg_x[3] & ~(0xFF << n)) | (value << n);
                video->bg_x[3] = v;
                video->bg_x_int[3] = GBAVideo::DecodeGBAFloat32(v);
                break;
            }
            case BG2Y:
            case BG2Y+1:
            case BG2Y+2:
            case BG2Y+3:
            {
                int n = (internal_offset - BG2Y) * 8;
                u32 v = (video->bg_y[2] & ~(0xFF << n)) | (value << n);
                video->bg_y[2] = v;
                video->bg_y_int[2] = GBAVideo::DecodeGBAFloat32(v);
                break;
            }
            case BG3Y:
            case BG3Y+1:
            case BG3Y+2:
            case BG3Y+3:
            {
                int n = (internal_offset - BG3Y) * 8;
                u32 v = (video->bg_y[3] & ~(0xFF << n)) | (value << n);
                video->bg_y[3] = v;
                video->bg_y_int[3] = GBAVideo::DecodeGBAFloat32(v);
                break;
            }
            case BG2PA:
            case BG2PA+1:
            {
                int n = (internal_offset - BG2PA) * 8;
                video->bg_pa[2] = (video->bg_pa[2] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PA:
            case BG3PA+1:
            {
                int n = (internal_offset - BG3PA) * 8;
                video->bg_pa[3] = (video->bg_pa[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG2PB:
            case BG2PB+1:
            {
                int n = (internal_offset - BG2PB) * 8;
                video->bg_pb[2] = (video->bg_pb[2] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PB:
            case BG3PB+1:
            {
                int n = (internal_offset - BG3PB) * 8;
                video->bg_pb[3] = (video->bg_pb[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG2PC:
            case BG2PC+1:
            {
                int n = (internal_offset - BG2PC) * 8;
                video->bg_pc[2] = (video->bg_pc[2] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PC:
            case BG3PC+1:
            {
                int n = (internal_offset - BG3PC) * 8;
                video->bg_pc[3] = (video->bg_pc[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case BG3PD:
            case BG3PD+1:
            {
                int n = (internal_offset - BG3PD) * 8;
                video->bg_pd[3] = (video->bg_pd[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case WIN0H:
                video->win_right[0] = value;
                break;
            case WIN0H+1:
                video->win_left[0] = value;
                break;
            case WIN1H:
                video->win_right[1] = value;
                break;
            case WIN1H+1:
                video->win_left[1] = value;
                break;
            case WIN0V:
                video->win_bottom[0] = value;
                break;
            case WIN0V+1:
                video->win_top[0] = value;
                break;
            case WIN1V:
                video->win_bottom[1] = value;
                break;
            case WIN1V+1:
                video->win_top[1] = value;
                break;
            case WININ:
                video->bg_winin[0][0] = value & 1;
                video->bg_winin[0][1] = value & 2;
                video->bg_winin[0][2] = value & 4;
                video->bg_winin[0][3] = value & 8;
                video->obj_winin[0] = value & 16;
                video->sfx_winin[0] = value & 32;
                break;
            case WININ+1:
                video->bg_winin[1][0] = value & 1;
                video->bg_winin[1][1] = value & 2;
                video->bg_winin[1][2] = value & 4;
                video->bg_winin[1][3] = value & 8;
                video->obj_winin[1] = value & 16;
                video->sfx_winin[1] = value & 32;
                break;
            case WINOUT:
                video->bg_winout[0] = value & 1;
                video->bg_winout[1] = value & 2;
                video->bg_winout[2] = value & 4;
                video->bg_winout[3] = value & 8;
                video->obj_winout = value & 16;
                video->sfx_winout = value & 32;
                break;
            case WINOUT+1:
                // TODO: OBJWIN
                break;
            case DMA0SAD:
            case DMA0SAD+1:
            case DMA0SAD+2:
            case DMA0SAD+3:
            {
                int n = (internal_offset - DMA0SAD) * 8;
                dma[0].source = (dma[0].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1SAD:
            case DMA1SAD+1:
            case DMA1SAD+2:
            case DMA1SAD+3:
            {
                int n = (internal_offset - DMA1SAD) * 8;
                dma[1].source = (dma[1].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2SAD:
            case DMA2SAD+1:
            case DMA2SAD+2:
            case DMA2SAD+3:
            {
                int n = (internal_offset - DMA2SAD) * 8;
                dma[2].source = (dma[2].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3SAD:
            case DMA3SAD+1:
            case DMA3SAD+2:
            case DMA3SAD+3:
            {
                int n = (internal_offset - DMA3SAD) * 8;
                dma[3].source = (dma[3].source & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0DAD:
            case DMA0DAD+1:
            case DMA0DAD+2:
            case DMA0DAD+3:
            {
                int n = (internal_offset - DMA0DAD) * 8;
                dma[0].dest = (dma[0].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1DAD:
            case DMA1DAD+1:
            case DMA1DAD+2:
            case DMA1DAD+3:
            {
                int n = (internal_offset - DMA1DAD) * 8;
                dma[1].dest = (dma[1].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2DAD:
            case DMA2DAD+1:
            case DMA2DAD+2:
            case DMA2DAD+3:
            {
                int n = (internal_offset - DMA2DAD) * 8;
                dma[2].dest = (dma[2].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3DAD:
            case DMA3DAD+1:
            case DMA3DAD+2:
            case DMA3DAD+3:
            {
                int n = (internal_offset - DMA3DAD) * 8;
                dma[3].dest = (dma[3].dest & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0CNT_L:
                dma[0].count = (dma[0].count & 0xFF00) | value;
                break;
            case DMA0CNT_L+1:
                dma[0].count = (dma[0].count & 0x00FF) | (value << 8);
                break;
            case DMA1CNT_L:
                dma[1].count = (dma[1].count & 0xFF00) | value;
                break;
            case DMA1CNT_L+1:
                dma[1].count = (dma[1].count & 0x00FF) | (value << 8);
                break;
            case DMA2CNT_L:
                dma[2].count = (dma[2].count & 0xFF00) | value;
                break;
            case DMA2CNT_L+1:
                dma[2].count = (dma[2].count & 0x00FF) | (value << 8);
                break;
            case DMA3CNT_L:
                dma[3].count = (dma[3].count & 0xFF00) | value;
                break;
            case DMA3CNT_L+1:
                dma[3].count = (dma[3].count & 0x00FF) | (value << 8);
                break;
            case DMA0CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(dma[0].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                dma[0].source_control = static_cast<AddressControl>(source_control);
                dma[0].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA0CNT_H+1:
            {
                int source_control = static_cast<int>(dma[0].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                dma[0].source_control = static_cast<AddressControl>(source_control);
                dma[0].repeat = value & 2;
                dma[0].size = static_cast<TransferSize>((value >> 2) & 1);
                dma[0].gamepack_drq = value & 8;
                dma[0].start_time = static_cast<StartTime>((value >> 4) & 3);
                dma[0].interrupt = value & 64;
                dma[0].enable = value & 128;
            
                if (dma[0].enable) 
                {
                    dma[0].source_int = dma[0].source & dma_source_mask[0];
                    dma[0].dest_int = dma[0].dest & dma_dest_mask[0];
                    dma[0].count_int = dma[0].count & dma_count_mask[0];

                    if (dma[0].count_int == 0) 
                        dma[0].count_int = dma_count_mask[0] + 1;
                }
                break;
            }
            case DMA1CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(dma[1].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                dma[1].source_control = static_cast<AddressControl>(source_control);
                dma[1].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA1CNT_H+1:
            {
                int source_control = static_cast<int>(dma[1].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                dma[1].source_control = static_cast<AddressControl>(source_control);
                dma[1].repeat = value & 2;
                dma[1].size = static_cast<TransferSize>((value >> 2) & 1);
                dma[1].gamepack_drq = value & 8;
                dma[1].start_time = static_cast<StartTime>((value >> 4) & 3);
                dma[1].interrupt = value & 64;
                dma[1].enable = value & 128;
                
                if (dma[1].enable) 
                {
                    dma[1].source_int = dma[1].source & dma_source_mask[1];
                    dma[1].dest_int = dma[1].dest & dma_dest_mask[1];
                    dma[1].count_int = dma[1].count & dma_count_mask[1];

                    if (dma[1].count_int == 0) 
                        dma[1].count_int = dma_count_mask[1] + 1;
                }
                break;
            }
            case DMA2CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(dma[2].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                dma[2].source_control = static_cast<AddressControl>(source_control);
                dma[2].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA2CNT_H+1:
            {
                int source_control = static_cast<int>(dma[2].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                dma[2].source_control = static_cast<AddressControl>(source_control);
                dma[2].repeat = value & 2;
                dma[2].size = static_cast<TransferSize>((value >> 2) & 1);
                dma[2].gamepack_drq = value & 8;
                dma[2].start_time = static_cast<StartTime>((value >> 4) & 3);
                dma[2].interrupt = value & 64;
                dma[2].enable = value & 128;
                
                if (dma[2].enable) 
                {
                    dma[2].source_int = dma[2].source & dma_source_mask[2];
                    dma[2].dest_int = dma[2].dest & dma_dest_mask[2];
                    dma[2].count_int = dma[2].count & dma_count_mask[2];

                    if (dma[2].count_int == 0) 
                        dma[2].count_int = dma_count_mask[2] + 1;
                }
                break;
            }
            case DMA3CNT_H:
            {
                // the upper bit is in the next byte...
                int source_control = static_cast<int>(dma[3].source_control);
                source_control = (source_control & 2) | ((value >> 7) & 1);
                dma[3].source_control = static_cast<AddressControl>(source_control);
                dma[3].dest_control = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA3CNT_H+1:
            {
                int source_control = static_cast<int>(dma[3].source_control);
                source_control = (source_control & ~3) | ((value & 1) << 1);
                dma[3].source_control = static_cast<AddressControl>(source_control);
                dma[3].repeat = value & 2;
                dma[3].size = static_cast<TransferSize>((value >> 2) & 1);
                dma[3].gamepack_drq = value & 8;
                dma[3].start_time = static_cast<StartTime>((value >> 4) & 3);
                dma[3].interrupt = value & 64;
                dma[3].enable = value & 128;
                
                if (dma[3].enable) 
                {
                    dma[3].source_int = dma[3].source & dma_source_mask[3];
                    dma[3].dest_int = dma[3].dest & dma_dest_mask[3];
                    dma[3].count_int = dma[3].count & dma_count_mask[3];

                    if (dma[3].count_int == 0) 
                        dma[3].count_int = dma_count_mask[3] + 1;
                }
                break;
            }
            case TM0CNT_L:
                timer[0].reload = (timer[0].reload & 0xFF00) | value;
                break;
            case TM0CNT_L+1:
                timer[0].reload = (timer[0].reload & 0x00FF) | (value << 8);
                break;
            case TM1CNT_L:
                timer[1].reload = (timer[1].reload & 0xFF00) | value;
                break;
            case TM1CNT_L+1:
                timer[1].reload = (timer[1].reload & 0x00FF) | (value << 8);
                break;
            case TM2CNT_L:
                timer[2].reload = (timer[2].reload & 0xFF00) | value;
                break;
            case TM2CNT_L+1:
                timer[2].reload = (timer[2].reload & 0x00FF) | (value << 8);
                break;
            case TM3CNT_L:
                timer[3].reload = (timer[3].reload & 0xFF00) | value;
                break;
            case TM3CNT_L+1:
                timer[3].reload = (timer[3].reload & 0x00FF) | (value << 8);
                break;
             case TM0CNT_H:
             case TM1CNT_H:
             case TM2CNT_H:
             case TM3CNT_H:
             {
                int n = (internal_offset - TM0CNT_H) / 4;
                timer[n].clock = value & 3;
                timer[n].countup = value & 4;
                timer[n].interrupt = value & 64;
                timer[n].enable = value & 128;
                break;
             }
            case IE:
                interrupt->ie = (interrupt->ie & 0xFF00) | value;
                break;
            case IE+1:
                interrupt->ie = (interrupt->ie & 0x00FF) | (value << 8);
                break;
            case IF:
                interrupt->if_ &= ~value;
                break;
            case IF+1:
                interrupt->if_ &= ~(value << 8);
                break;
            case WAITCNT:
                waitstate.sram = value & 3;
                waitstate.first[0] = (value >> 2) & 3;
                waitstate.second[0] = (value >> 4) & 1;
                waitstate.first[1] = (value >> 5) & 3;
                waitstate.second[1] = value >> 7;
                break;
            case WAITCNT+1:
                waitstate.first[2] = value & 3;
                waitstate.second[2] = (value >> 2) & 1;
                waitstate.prefetch = value & 64;
                break;
            case IME:
                interrupt->ime = (interrupt->ime & 0xFF00) | value;
                break;
            case IME+1:
                interrupt->ime = (interrupt->ime & 0x00FF) | (value << 8);
                break;
            case HALTCNT:
                halt_state = (value & 0x80) ? HaltState::Stop : HaltState::Halt;
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
            backup->WriteByte(offset, value); 
            break;
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write to invalid/unimplemented address (0x%x)", offset);
            #endif           
            break;
        }
    }

    void GBAMemory::WriteHWord(u32 offset, u16 value)
    {
        int page = (offset >> 24) & 0xF;
        u32 internal_offset = offset & 0xFFFFFF;

        switch (page)
        {
        case 5:
            video->pal[internal_offset % 0x400] = value & 0xFF;
            video->pal[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            return;
        case 6:
            internal_offset %= 0x20000;

            if (internal_offset >= 0x18000)
                internal_offset -= 0x8000;
            
            video->vram[internal_offset] = value & 0xFF;
            video->vram[internal_offset + 1] = (value >> 8) & 0xFF;
            return;
        case 7:
            video->obj[internal_offset % 0x400] = value & 0xFF;
            video->obj[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            return;
        default:
            WriteByte(offset, value & 0xFF);
            WriteByte(offset + 1, (value >> 8) & 0xFF);
            return;
        }
    }

    void GBAMemory::WriteWord(u32 offset, u32 value)
    {
        WriteHWord(offset, value & 0xFFFF);
        WriteHWord(offset + 2, (value >> 16) & 0xFFFF);
        
        // Handle special timer behaviour
        if (value & (1 << 23))
        {
            switch (offset) 
            {
            case TM0CNT_L|0x04000000:
                timer[0].count = timer[0].reload;
                break;
            case TM1CNT_L|0x04000000:
                timer[1].count = timer[1].reload;
                break;
            case TM2CNT_L|0x04000000:
                timer[2].count = timer[2].reload;
                break;
            case TM3CNT_L|0x04000000:
                timer[3].count = timer[3].reload;
                break;
            }
        }
    }

}
