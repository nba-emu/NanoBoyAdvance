/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <cstdint>

namespace GameBoyAdvance {

class CPU;

class DMAController {
public:
    DMAController(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    auto Read(int id, int offset) -> std::uint8_t;
    void Write(int id, int offset, std::uint8_t value);
    void RequestHBlank();
    void RequestVBlank();
    void RequestFIFO(int fifo);
    void Run();
    
    bool IsRunning() const { return run_set != 0; }
    
private:
    void MarkDMAForExecution(int id);
    void RunFIFO();
    
    CPU* cpu;

    int  hblank_set;
    int  vblank_set;
    int  run_set;
    int  current;
    bool interleaved;
    
    enum AddressControl  {
        DMA_INCREMENT = 0,
        DMA_DECREMENT = 1,
        DMA_FIXED     = 2,
        DMA_RELOAD    = 3
    };

    enum StartTiming {
        DMA_IMMEDIATE = 0,
        DMA_VBLANK    = 1,
        DMA_HBLANK    = 2,
        DMA_SPECIAL   = 3
    };

    enum WordSize {
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
        AddressControl dst_cntl;
        AddressControl src_cntl;
        StartTiming time;
        WordSize size;

        struct Internal {
            int request_count;
            
            std::uint32_t length;
            std::uint32_t dst_addr;
            std::uint32_t src_addr;
        } internal;
    } dma[4];
};

}