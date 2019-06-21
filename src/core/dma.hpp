/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance::GBA {

class CPU;

class DMAController {
public:
    DMAController(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    auto Read(int id, int offset) -> std::uint8_t;
    void Write(int id, int offset, std::uint8_t value);
    void TriggerHBlankDMA();
    void TriggerVBlankDMA();
    void Run();
    
    bool IsRunning() const {
        return dma_run_set != 0;
    }
    
private:
    void MarkDMAForExecution(int id);
    
    CPU* cpu;
    
    /* TODO: rename these! */
    int  dma_hblank_mask;
    int  dma_vblank_mask;
    int  dma_run_set;
    int  dma_current;
    bool dma_interleaved;
    
    enum DMAControl {
        DMA_INCREMENT = 0,
        DMA_DECREMENT = 1,
        DMA_FIXED     = 2,
        DMA_RELOAD    = 3
    };

    enum DMATime {
        DMA_IMMEDIATE = 0,
        DMA_VBLANK    = 1,
        DMA_HBLANK    = 2,
        DMA_SPECIAL   = 3
    };

    enum DMASize {
        DMA_HWORD = 0,
        DMA_WORD  = 1
    };
    
    struct DMA {
        bool enable;
        bool repeat;
        bool interrupt;
        bool gamepak;

        std::uint16_t length;
        std::uint32_t dst_addr;
        std::uint32_t src_addr;
        DMAControl dst_cntl;
        DMAControl src_cntl;
        DMATime time;
        DMASize size;

        struct Internal {
            std::uint32_t length;
            std::uint32_t dst_addr;
            std::uint32_t src_addr;
        } internal;
    } dma[4];
    
    static constexpr std::uint32_t s_dma_dst_mask[4] = { 0x07FFFFFF, 0x07FFFFFF, 0x07FFFFFF, 0x0FFFFFFF };
    static constexpr std::uint32_t s_dma_src_mask[4] = { 0x07FFFFFF, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF };
    static constexpr std::uint32_t s_dma_len_mask[4] = { 0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF };
};

}