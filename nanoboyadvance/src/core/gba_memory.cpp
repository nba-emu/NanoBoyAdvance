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

#include "gba_memory.h"
#include "common/log.h"
#include "common/file.h"

/**
 * TODO: Refactor code for more beauty and check if memory busses
 * are handled correct accordingly to GBATEK. Also pass invalid=true to the 
 * debugger when accessing memory out of bounds of a valid page. 
 */

namespace NanoboyAdvance
{
    GBAMemory::GBAMemory(string bios_file, string rom_file, string save_file)
    {
        bool found_save_type = false;

        // Read files and init hardware
        bios = File::ReadFile(bios_file);
        rom = File::ReadFile(rom_file);
        rom_size = File::GetFileSize(rom_file);
        gba_io = (GBAIO*)io;
        interrupt = new GBAInterrupt();
        interrupt->ie = 0;
        interrupt->if_ = 0;
        interrupt->ime = 0;
        timer = new GBATimer(gba_io);
        video = new GBAVideo(interrupt);
        
        // Reset debug hook
        memory_hook = NULL;

        // Init some memory
        memset(wram, 0, 0x40000);
        memset(iram, 0, 0x8000);
        memset(io, 0, 0x3FF);
        memset(sram, 0, 0x10000);
        io[0x130] = 0xFF;
        
        // Detect savetype
        for (int i = 0; i < rom_size; i += 4)
        {
            if (memcmp(rom + i, "EEPROM_V", 8) == 0)
            {
                save_type = GBASaveType::EEPROM;
                found_save_type = true;
                cout << "EEPROM" << endl;
            }
            else if (memcmp(rom + i, "SRAM_V", 6) == 0)
            {
                save_type = GBASaveType::SRAM;
                found_save_type = true;
                cout << "SRAM" << endl;
            }
            else if (memcmp(rom + i, "FLASH_V", 7) == 0)
            {
                save_type = GBASaveType::FLASH64;
                found_save_type = true;
                cout << "FLASH" << endl;
                backup = new GBAFlash(save_file, false);
            }
            else if (memcmp(rom + i, "FLASH512_V", 10) == 0)
            {
                save_type = GBASaveType::FLASH64;
                found_save_type = true;
                cout << "FLASH512" << endl;
                backup = new GBAFlash(save_file, false);
            }
            else if (memcmp(rom + i, "FLASH1M_V", 9) == 0)
            {
                save_type = GBASaveType::FLASH128;
                found_save_type = true;
                cout << "FLASH1M" << endl;
                backup = new GBAFlash(save_file, true);
            }
        }

        // Log if we could't determine the savetype
        if (!found_save_type)
        {
            cout << "No savetype detected, defaulting to SRAM..." << endl;
        }
    }

    GBAMemory::~GBAMemory()
    {
        delete backup;
        delete timer;
        delete video;
        delete interrupt;
    }

    void GBAMemory::SetCallback(MemoryCallback callback)
    {
        memory_hook = callback;
    }
    
    void GBAMemory::Step()
    {
        // run though all dma channels
        for (int i = 0; i < 4; i++) {
            // is the current dma active?
            if (dma_enable[i]) {
                bool start = false;
                
                // Determine if DMA will be initiated
                switch (dma_time[i]) {
                // Immediatly
                case 0:
                    start = true;
                    break;
                // VBlank
                case 1:
                    if (video->vblank_dma) {
                        start = true;
                        video->vblank_dma = false;
                    }
                    break;
                // HBlank
                case 2:
                    if (video->hblank_dma) {
                        start = true;
                        video->hblank_dma = false;
                    }
                    break;
                // Special
                case 3:
                    // TODO: 1) Video Capture Mode
                    //       2) DMA1/2 Sound FIFO
                    if (i == 3) {
                        LOG(LOG_ERROR, "DMA: Video Capture Mode not supported.");
                    }
                    break;
                }
                
                // Run if determined so
                if (start) {
                    AddressControl dst_cntl = dma_dst_cntl[i];
                    AddressControl src_cntl = dma_src_cntl[i];
                    bool transfer_words = dma_words[i];

                    // DMA Debug Log
                    #if DEBUG
                        u32 value = ReadHWord(dma_src[i]);
                        LOG(LOG_INFO, "DMA%d: s=%x d=%x c=%x count=%x l=%d v=%x", i, dma_src_int[i], dma_dst_int[i], 0, dma_count_int[i], video->vcount, value);
                    #endif
                    
                    // Throw error if unsupported Game Pack DRQ is requested
                    if (dma_gp_drq[i]) {
                        LOG(LOG_ERROR, "Game Pak DRQ not supported.");
                    }
                                        
                    // TODO: FIFO A/B special transfer
                    // Run as long as there is data to transfer
                    while (dma_count_int[i] != 0) {
                        // Transfer either Word or HWord
                        if (transfer_words) {
                            WriteWord(dma_dst_int[i], ReadWord(dma_src_int[i]));
                        } else {
                            WriteHWord(dma_dst_int[i], ReadHWord(dma_src_int[i]));
                        }
                        
                        // Update destination address
                        if (dst_cntl == Increment || dst_cntl == IncrementAndReload) {
                            dma_dst_int[i] += transfer_words ? 4 : 2;
                        } else if (dst_cntl == Decrement) {
                            dma_dst_int[i] -= transfer_words ? 4 : 2;
                        }
                        
                        // Update source address
                        if (src_cntl == Increment || src_cntl == IncrementAndReload) {
                            dma_src_int[i] += transfer_words ? 4 : 2;
                        } else if (src_cntl == Decrement) {
                            dma_src_int[i] -= transfer_words ? 4 : 2;
                        }
                        
                        // Update count
                        dma_count_int[i]--;
                    }
                    
                    // Reload dma_count_int and dma_dst_int as specified
                    if (dma_repeat[i]) {
                        // TODO: Find a more beautiful(?) solution.
                        switch (i) {
                        case 0:
                            dma_count_int[0] = dma_count[0] & 0x3FFF;
                            if (dma_count_int[0] == 0) {
                                dma_count_int[0] = 0x4000;
                            }
                            if (dst_cntl == IncrementAndReload) {
                                dma_dst_int[0] = dma_dst[0] & 0x07FFFFFF;
                            }
                            break;
                        case 1:
                            dma_count_int[1] = dma_count[1] & 0x3FFF;
                            if (dma_count_int[1] == 0) {
                                dma_count_int[1] = 0x4000;
                            }
                            if (dst_cntl == IncrementAndReload) {
                                dma_dst_int[1] = dma_dst[1] & 0x07FFFFFF;
                            }
                            break;
                        case 2:
                            dma_count_int[2] = dma_count[2] & 0x3FFF;
                            if (dma_count_int[2] == 0) {
                                dma_count_int[2] = 0x4000;
                            }
                            if (dst_cntl == IncrementAndReload) {
                                dma_dst_int[2] = dma_dst[2] & 0x07FFFFFF;
                            }
                            break;
                        case 3:
                            dma_count_int[3] = dma_count[3];
                            if (dma_count_int[3] == 0) {
                                dma_count_int[3] = 0x10000;
                            }
                            if (dst_cntl == IncrementAndReload) {
                                dma_dst_int[3] = dma_dst[3] & 0x0FFFFFFF;
                            }
                            break;
                        }
                    } else {
                        dma_enable[i] = false;
                    }
                    
                    // Raise DMA IRQ if specified
                    if (dma_irq[i]) {
                        interrupt->if_ |= (1 << (8 + i));
                    }
                }
            }
        }
    }

    u8 GBAMemory::ReadByte(u32 offset)
    {
        int page = offset >> 24;
        bool invalid = false;
        u32 internal_offset = offset & 0xFFFFFF;        

        switch (page)
        {
        case 0:
        case 1:
            #ifdef DEBUG
            ASSERT(internal_offset >= 0x4000, LOG_ERROR, "BIOS read: offset out of bounds");
            #endif            
            if (internal_offset >= 0x4000)
            {
                return 0;
            }
            return bios[internal_offset];
        case 2:
            return wram[internal_offset % 0x40000];
        case 3:
            return iram[internal_offset % 0x8000];
        case 4:
            if ((internal_offset & 0xFFFF) == 0x800)
            {
                internal_offset &= 0xFFFF;
            }
            
            /*if (internal_offset >= 0x3FF)
            {
                #ifdef DEBUG
                LOG(LOG_ERROR, "IO read: offset out of bounds (0x%x)", offset); 
                #endif                
                return 0;
            }*/
            
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
                return (video->vblank_flag ? 1 : 0) |
                       (video->hblank_flag ? 2 : 0) |
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
                       (video->bg_pal_256[n] ? 128 : 0);
            }
            case BG0CNT+1:
            case BG1CNT+1:
            case BG2CNT+1:
            case BG3CNT+1:
            {
                int n = (internal_offset - BG0CNT - 1) / 2;
                return (video->bg_map_base[n] / 0x800) |
                       (video->bg_wraparound[n] ? 32 : 0) |
                       (video->bg_size[n] << 14);
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
            case IE:
                return interrupt->ie & 0xFF;
             case IE+1:
                return interrupt->ie >> 8;
            case IF:
                return interrupt->if_ & 0xFF;
            case IF+1:
                return interrupt->if_ >> 8;
            case IME:
                return interrupt->ime & 0xFF;
            case IME+1:
                return interrupt->ime >> 8;
            }
            //return 0;
            return io[internal_offset];
        case 5:
            return video->pal[internal_offset % 0x400];
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000)
            {
                internal_offset -= 0x8000;
            }
            return video->vram[internal_offset];
        case 7:
            return video->obj[internal_offset % 0x400];
        case 8:
            // TODO: Prevent out of bounds read, we should save the rom size somewhere
            return rom[internal_offset];
        case 9:
            // TODO: Prevent out of bounds read, we should save the rom size somewhere
            return rom[0x1000000 + internal_offset];
        case 0xE:
            if (save_type == GBASaveType::FLASH64 || save_type == GBASaveType::FLASH128)
            {
                return backup->ReadByte(offset);
            }
            return sram[internal_offset];
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Read from invalid/unimplemented address (0x%x)", offset);
            #endif            
            invalid = true;
            break;
        }
        MemoryHook(offset, false, invalid);
        return 0;
    }

    u16 GBAMemory::ReadHWord(u32 offset)
    {
        // TODO: Handle special case SRAM
        return ReadByte(offset) | (ReadByte(offset + 1) << 8);
    }

    u32 GBAMemory::ReadWord(u32 offset)
    {
        // TODO: Handle special case SRAM
        return ReadByte(offset) | (ReadByte(offset + 1) << 8) | (ReadByte(offset + 2) << 16) | (ReadByte(offset + 3) << 24);
    }

    void GBAMemory::WriteByte(u32 offset, u8 value)
    {
        int page = offset >> 24;
        u32 internal_offset = offset & 0xFFFFFF;
        bool invalid = false;

        switch (page) {
        case 0:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write into BIOS memory not allowed (0x%x)", offset);
            #endif
            invalid = true;            
            break;
        case 2:
            wram[internal_offset % 0x40000] = value;
            break;
        case 3:
            iram[internal_offset % 0x8000] = value;
            break;
        case 4:
        {
            bool write = true;

            // If the address it out of bounds we should exit now
            if (internal_offset >= 0x3FF && (internal_offset & 0xFFFF) != 0x800) {
                #ifdef DEBUG
                LOG(LOG_ERROR, "IO write: offset out of bounds (0x%x)", offset);
                #endif
                break;
            }

            // Emulate IO mirror at 04xx0800
            if ((internal_offset & 0xFFFF) == 0x800) {
                internal_offset &= 0xFFFF;
            }

            // Writing to some registers causes special behaviour which must be emulated
            switch (internal_offset) {
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
                video->bg_wraparound[n] = value & 32;
                video->bg_size[n] = value >> 14;
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
                video->bg_hofs[n] = (video->bg_hofs[n] & 0xFF) | (value << 8);
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
                video->bg_vofs[n] = (video->bg_vofs[n] & 0xFF) | (value << 8);
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
                dma_src[0] = (dma_src[0] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1SAD:
            case DMA1SAD+1:
            case DMA1SAD+2:
            case DMA1SAD+3:
            {
                int n = (internal_offset - DMA1SAD) * 8;
                dma_src[1] = (dma_src[1] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2SAD:
            case DMA2SAD+1:
            case DMA2SAD+2:
            case DMA2SAD+3:
            {
                int n = (internal_offset - DMA2SAD) * 8;
                dma_src[2] = (dma_src[2] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3SAD:
            case DMA3SAD+1:
            case DMA3SAD+2:
            case DMA3SAD+3:
            {
                int n = (internal_offset - DMA3SAD) * 8;
                dma_src[3] = (dma_src[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0DAD:
            case DMA0DAD+1:
            case DMA0DAD+2:
            case DMA0DAD+3:
            {
                int n = (internal_offset - DMA0DAD) * 8;
                dma_dst[0] = (dma_dst[0] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA1DAD:
            case DMA1DAD+1:
            case DMA1DAD+2:
            case DMA1DAD+3:
            {
                int n = (internal_offset - DMA1DAD) * 8;
                dma_dst[1] = (dma_dst[1] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA2DAD:
            case DMA2DAD+1:
            case DMA2DAD+2:
            case DMA2DAD+3:
            {
                int n = (internal_offset - DMA2DAD) * 8;
                dma_dst[2] = (dma_dst[2] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA3DAD:
            case DMA3DAD+1:
            case DMA3DAD+2:
            case DMA3DAD+3:
            {
                int n = (internal_offset - DMA3DAD) * 8;
                dma_dst[3] = (dma_dst[3] & ~(0xFF << n)) | (value << n);
                break;
            }
            case DMA0CNT_L:
                dma_count[0] = (dma_count[0] & 0xFF00) | value;
                break;
            case DMA0CNT_L+1:
                dma_count[0] = (dma_count[0] & 0x00FF) | (value << 8);
                break;
            case DMA1CNT_L:
                dma_count[1] = (dma_count[1] & 0xFF00) | value;
                break;
            case DMA1CNT_L+1:
                dma_count[1] = (dma_count[1] & 0x00FF) | (value << 8);
                break;
            case DMA2CNT_L:
                dma_count[2] = (dma_count[2] & 0xFF00) | value;
                break;
            case DMA2CNT_L+1:
                dma_count[2] = (dma_count[2] & 0x00FF) | (value << 8);
                break;
            case DMA3CNT_L:
                dma_count[3] = (dma_count[3] & 0xFF00) | value;
                break;
            case DMA3CNT_L+1:
                dma_count[3] = (dma_count[3] & 0x00FF) | (value << 8);
                break;
            case DMA0CNT_H:
            {
                // the upper bit is in the next byte...
                int src_cntl = static_cast<int>(dma_src_cntl[0]);
                src_cntl = (src_cntl & 2) | ((value >> 7) & 1);
                dma_src_cntl[0] = static_cast<AddressControl>(src_cntl);
                dma_dst_cntl[0] = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA0CNT_H+1:
            {
                int src_cntl = static_cast<int>(dma_src_cntl[0]);
                src_cntl = (src_cntl & ~3) | ((value & 1) << 1);
                dma_src_cntl[0] = static_cast<AddressControl>(src_cntl);
                dma_repeat[0] = value & 2;
                dma_words[0] = value & 4;
                dma_gp_drq[0] = value & 8;
                dma_time[0] = (value >> 4) & 3;
                dma_irq[0] = value & 64;
                dma_enable[0] = value & 128;
            
                if (value & 128) {
                    dma_src_int[0] = dma_src[0] & 0x07FFFFFF;
                    dma_dst_int[0] = dma_dst[0] & 0x07FFFFFF;
                    dma_count_int[0] = dma_count[0] & 0x3FFF;
                    if (dma_count_int[0] == 0) {
                        dma_count_int[0] = 0x4000;
                    }
                }
                break;
            }
            case DMA1CNT_H:
            {
                // the upper bit is in the next byte...
                int src_cntl = static_cast<int>(dma_src_cntl[1]);
                src_cntl = (src_cntl & 2) | ((value >> 7) & 1);
                dma_src_cntl[1] = static_cast<AddressControl>(src_cntl);
                dma_dst_cntl[1] = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA1CNT_H+1:
            {
                int src_cntl = static_cast<int>(dma_src_cntl[1]);
                src_cntl = (src_cntl & ~3) | ((value & 1) << 1);
                dma_src_cntl[1] = static_cast<AddressControl>(src_cntl);
                dma_repeat[1] = value & 2;
                dma_words[1] = value & 4;
                dma_gp_drq[1] = value & 8;
                dma_time[1] = (value >> 4) & 3;
                dma_irq[1] = value & 64;
                dma_enable[1] = value & 128;
                
                if (value & 128) {
                    dma_src_int[1] = dma_src[1] & 0x0FFFFFFF;
                    dma_dst_int[1] = dma_dst[1] & 0x07FFFFFF;
                    dma_count_int[1] = dma_count[1] & 0x3FFF;
                    if (dma_count_int[1] == 0) {
                        dma_count_int[1] = 0x4000;
                    }
                }
                break;
            }
            case DMA2CNT_H:
            {
                // the upper bit is in the next byte...
                int src_cntl = static_cast<int>(dma_src_cntl[2]);
                src_cntl = (src_cntl & 2) | ((value >> 7) & 1);
                dma_src_cntl[2] = static_cast<AddressControl>(src_cntl);
                dma_dst_cntl[2] = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA2CNT_H+1:
            {
                int src_cntl = static_cast<int>(dma_src_cntl[2]);
                src_cntl = (src_cntl & ~3) | ((value & 1) << 1);
                dma_src_cntl[2] = static_cast<AddressControl>(src_cntl);
                dma_repeat[2] = value & 2;
                dma_words[2] = value & 4;
                dma_gp_drq[2] = value & 8;
                dma_time[2] = (value >> 4) & 3;
                dma_irq[2] = value & 64;
                dma_enable[2] = value & 128;
                
                if (value & 128) {
                    dma_src_int[2] = dma_src[2] & 0x0FFFFFFF;
                    dma_dst_int[2] = dma_dst[2] & 0x07FFFFFF;
                    dma_count_int[2] = dma_count[2] & 0x3FFF;
                    if (dma_count_int[2] == 0) {
                        dma_count_int[2] = 0x4000;
                    }
                }
                break;
            }
            case DMA3CNT_H:
            {
                // the upper bit is in the next byte...
                int src_cntl = static_cast<int>(dma_src_cntl[3]);
                src_cntl = (src_cntl & 2) | ((value >> 7) & 1);
                dma_src_cntl[3] = static_cast<AddressControl>(src_cntl);
                dma_dst_cntl[3] = static_cast<AddressControl>((value >> 5) & 3);
                break;
            }
            case DMA3CNT_H+1:
            {
                int src_cntl = static_cast<int>(dma_src_cntl[3]);
                src_cntl = (src_cntl & ~3) | ((value & 1) << 1);
                dma_src_cntl[3] = static_cast<AddressControl>(src_cntl);
                dma_repeat[3] = value & 2;
                dma_words[3] = value & 4;
                dma_gp_drq[3] = value & 8;
                dma_time[3] = (value >> 4) & 3;
                dma_irq[3] = value & 64;
                dma_enable[3] = value & 128;
                
                if (value & 128) {
                    dma_src_int[3] = dma_src[3] & 0x0FFFFFFF;
                    dma_dst_int[3] = dma_dst[3] & 0x0FFFFFFF;
                    dma_count_int[3] = dma_count[3];
                    if (dma_count_int[3] == 0) {
                        dma_count_int[3] = 0x10000;
                    }
                }
                break;
            }
            case TM0CNT_L:
                timer->timer_reload[0] = (timer->timer_reload[0]  & 0xFF00) | value;
                write = false;
                break;
            case TM0CNT_L+1:
                timer->timer_reload[0] = (timer->timer_reload[0] & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM0CNT_H:
            case TM0CNT_H+1:
                timer->timer0_altered = true;
                break;
            case TM1CNT_L:
                timer->timer_reload[1]  = (timer->timer_reload[1] & 0xFF00) | value;
                write = false;
                break;
            case TM1CNT_L+1:
                timer->timer_reload[1] = (timer->timer_reload[1] & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM1CNT_H:
            case TM1CNT_H+1:
                timer->timer1_altered = true;
                break;
            case TM2CNT_L:
                timer->timer_reload[2] = (timer->timer_reload[2] & 0xFF00) | value;
                write = false;
                break;
            case TM2CNT_L+1:
                timer->timer_reload[2] = (timer->timer_reload[2] & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM2CNT_H:
            case TM2CNT_H+1:
                timer->timer2_altered = true;
                break;
            case TM3CNT_L:
                timer->timer_reload[3] = (timer->timer_reload[3] & 0xFF00) | value;
                write = false;
                break;
            case TM3CNT_L+1:
                timer->timer_reload[3] = (timer->timer_reload[3] & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM3CNT_H:
            case TM3CNT_H+1:
                timer->timer3_altered = true;
                break;
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
            case IME:
                interrupt->ime = (interrupt->ime & 0xFF00) | value;
                break;
            case IME+1:
                interrupt->ime = (interrupt->ime & 0x00FF) | (value << 8);
                break;
            case HALTCNT:
                halt_state = (value & 0x80) ? GBAHaltState::Stop : GBAHaltState::Halt;
                break;
            }
            break;
        }
        case 5:
        case 6:
        case 7:
            // We cannot write a single byte. Therefore the byte will be duplicated in the data bus and a halfword write will be performed
            WriteHWord(offset & ~1, (value << 8) | value);
            break;
        case 8:
        case 9:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write into ROM memory not allowed (0x%x)", offset);
            #endif            
            invalid = true;            
            break;
        case 0xE:
            if (save_type == GBASaveType::FLASH64 || save_type == GBASaveType::FLASH128) {
                backup->WriteByte(offset, value);
                return;
            }
            sram[internal_offset] = value;            
            break;
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Write to invalid/unimplemented address (0x%x)", offset);
            #endif
            invalid = true;            
            break;
        }
        MemoryHook(offset, true, invalid);
    }

    void GBAMemory::WriteHWord(u32 offset, u16 value)
    {
        int page = (offset >> 24) & 0xF;
        u32 internal_offset = offset & 0xFFFFFF;

        switch (page) {
        case 5:
            video->pal[internal_offset % 0x400] = value & 0xFF;
            video->pal[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            break;
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000) {
                internal_offset -= 0x8000;
            }
            video->vram[internal_offset] = value & 0xFF;
            video->vram[internal_offset + 1] = (value >> 8) & 0xFF;
            break;
        case 7:
            video->obj[internal_offset % 0x400] = value & 0xFF;
            video->obj[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            break;
        default:
            WriteByte(offset, value & 0xFF);
            WriteByte(offset + 1, (value >> 8) & 0xFF);
            return; // bypass MemoryHook call
        }
        MemoryHook(offset, true, false);
    }

    void GBAMemory::WriteWord(u32 offset, u32 value)
    {
        if (value & (1 << 23)) {
            switch (offset) {
            case 0x04000100:
                gba_io->tm0cnt_l = value & 0xFFFF;
                break;
            case 0x04000104:
                gba_io->tm1cnt_l = value & 0xFFFF;
                break;
            case 0x04000108:
                gba_io->tm2cnt_l = value & 0xFFFF;
                break;
            case 0x0400010C:
                gba_io->tm3cnt_l = value & 0xFFFF;
                break;
            }
        }
        WriteHWord(offset, value & 0xFFFF);
        WriteHWord(offset + 2, (value >> 16) & 0xFFFF);
    }

}
