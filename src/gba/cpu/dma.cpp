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

#include "cpu.hpp"
#include "dma.hpp"
#include "mmio.hpp"

using namespace GameBoyAdvance;

static constexpr std::uint32_t g_dma_dst_mask[4] = { 0x07FFFFFF, 0x07FFFFFF, 0x07FFFFFF, 0x0FFFFFFF };
static constexpr std::uint32_t g_dma_src_mask[4] = { 0x07FFFFFF, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF };
static constexpr std::uint32_t g_dma_len_mask[4] = { 0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF };

/* TODO: what happens if src_cntl equals DMA_RELOAD? */
static constexpr int g_dma_modify[2][4] = {
  { 2, -2, 0, 2 },
  { 4, -4, 0, 4 }
};

/* Retrieves DMA with highest priority from a DMA bitset. */
static constexpr int g_dma_from_bitset[] = {
  /* 0b0000 */ -1,
  /* 0b0001 */  0,
  /* 0b0010 */  1,
  /* 0b0011 */  0,
  /* 0b0100 */  2,
  /* 0b0101 */  0,
  /* 0b0110 */  1,
  /* 0b0111 */  0,
  /* 0b1000 */  3,
  /* 0b1001 */  0,
  /* 0b1010 */  1,
  /* 0b1011 */  0,
  /* 0b1100 */  2,
  /* 0b1101 */  0,
  /* 0b1110 */  1,
  /* 0b1111 */  0
};

void DMAController::Reset() {
  hblank_set = 0;
  vblank_set = 0;
  run_set = 0;
  current = 0;
  interleaved = false;
  
  for (int id = 0; id < 4; id++) {
    dma[id].enable    = false;
    dma[id].repeat    = false;
    dma[id].interrupt = false;
    dma[id].gamepak   = false;
    dma[id].length    = 0;
    dma[id].dst_addr  = 0;
    dma[id].src_addr  = 0;
    dma[id].internal.length   = 0;
    dma[id].internal.dst_addr = 0;
    dma[id].internal.src_addr = 0;
    dma[id].size     = DMA_HWORD;
    dma[id].time     = DMA_IMMEDIATE;
    dma[id].dst_cntl = DMA_INCREMENT;
    dma[id].src_cntl = DMA_INCREMENT;
  }
}

void DMAController::MarkDMAForExecution(int id) {
  /* If no other DMA is running or this DMA has higher priority
   * then execute this DMA directly.
   * Lower priority DMAs will be interleaved in the latter case.
   */
  if (run_set == 0) {
    current = id;
  } else if (id < current) {
    current = id;
    interleaved = true;
  }

  run_set |= (1 << id);
}

void DMAController::RequestHBlank() {
  int hblank_dma = g_dma_from_bitset[hblank_set];
  
  if (hblank_dma >= 0) {
    MarkDMAForExecution(hblank_dma);
  }
}

void DMAController::RequestVBlank() {
  int vblank_dma = g_dma_from_bitset[vblank_set];
  
  if (vblank_dma >= 0) {
    MarkDMAForExecution(vblank_dma);
  }
}

void DMAController::RequestFIFO(int fifo) {
  std::uint32_t address[2] = { FIFO_A, FIFO_B };
  
  for (int id = 1; id <= 2; id++) {
    if (dma[id].enable &&
        dma[id].time == DMA_SPECIAL &&
        dma[id].dst_addr == address[fifo])
    {
      dma[id].internal.request_count++;
      MarkDMAForExecution(id);
      break;
    }
  }
}

void DMAController::Run() {
  auto& dma = this->dma[current];
  
  auto src_modify = g_dma_modify[dma.size][dma.src_cntl];
  auto dst_modify = g_dma_modify[dma.size][dma.dst_cntl];
  
  /* FIFO DMA does not work quite a like a normal DMA.
   * For example the repeat bit works different, word transfer is enforced
   * and if FIFO DMA was requested multiple times, it needs to run more than once.
   */
  if (dma.time == DMA_SPECIAL && (current == 1 || current == 2)) {
    RunFIFO();
    return;
  }
  
  std::uint32_t word;
  
  /* Run DMA until completion or interruption. */
  if (dma.size == DMA_WORD) {
    while (dma.internal.length != 0) {
      if (cpu->ticks_cpu_left <= 0) return;

      /* Stop if DMA was interleaved by higher priority DMA. */
      if (interleaved) {
        interleaved = false;
        return;
      }

      word = cpu->ReadWord(dma.internal.src_addr & ~3, ARM::ACCESS_SEQ);
      cpu->WriteWord(dma.internal.dst_addr & ~3, word, ARM::ACCESS_SEQ);

      dma.internal.src_addr += src_modify;
      dma.internal.dst_addr += dst_modify;
      dma.internal.length--;
    }
  } else {
    while (dma.internal.length != 0) {
      if (cpu->ticks_cpu_left <= 0) return;

      /* Stop if DMA was interleaved by higher priority DMA. */
      if (interleaved) {
        interleaved = false;
        return;
      }

      word = cpu->ReadHalf(dma.internal.src_addr & ~1, ARM::ACCESS_SEQ);
      cpu->WriteHalf(dma.internal.dst_addr & ~1, word, ARM::ACCESS_SEQ);
      
      dma.internal.src_addr += src_modify;
      dma.internal.dst_addr += dst_modify;
      dma.internal.length--;
    }
  }

  
  if (dma.interrupt) {
    cpu->mmio.irq_if |= CPU::INT_DMA0 << current;
  }
  
  if (dma.repeat) {
    /* Reload the internal length counter. */
    dma.internal.length = dma.length & g_dma_len_mask[current];
    if (dma.internal.length == 0) {
      dma.internal.length = g_dma_len_mask[current] + 1;
    }

    /* Reload destination address if specified. */
    if (dma.dst_cntl == DMA_RELOAD) {
      dma.internal.dst_addr = dma.dst_addr & g_dma_dst_mask[current];
    }

    /* If DMA is specified to be non-immediate, wait for it to be retriggered. */
    if (dma.time != DMA_IMMEDIATE) {
      run_set &= ~(1 << current);
    }
  } else {
    dma.enable = false;
    run_set &= ~(1 << current);
  }
  
  current = g_dma_from_bitset[run_set];
}

void DMAController::RunFIFO() {
  auto& dma = this->dma[current];
  
  std::uint32_t word;
  
  /* CHECKME: is cycle budget overshoot a problem here? */
  for (int i = 0; i < 4; i++) {
    word = cpu->ReadWord(dma.internal.src_addr & ~3, ARM::ACCESS_SEQ);
    cpu->WriteWord(dma.internal.dst_addr & ~3, word, ARM::ACCESS_SEQ);
    
    dma.internal.src_addr += 4;
  }
    
  if (--dma.internal.request_count == 0) {
    run_set &= ~(1 << current);
  }
    
  if (dma.interrupt) {
    cpu->mmio.irq_if |= CPU::INT_DMA0 << current;
  }
    
  current = g_dma_from_bitset[run_set];
}

auto DMAController::Read(int id, int offset) -> std::uint8_t {
  switch (offset) {
    /* DMAXCNT_H */
    case 10: {
      return (dma[id].dst_cntl << 5) |
             (dma[id].src_cntl << 7);
    }
    case 11: {
      return (dma[id].src_cntl  >> 1) |
             (dma[id].size      << 2) |
             (dma[id].time      << 4) |
             (dma[id].repeat    ? 2   : 0) |
             (dma[id].gamepak   ? 8   : 0) |
             (dma[id].interrupt ? 64  : 0) |
             (dma[id].enable    ? 128 : 0);
    }
    default: return 0;
  }
}

void DMAController::Write(int id, int offset, std::uint8_t value) {
  switch (offset) {
    /* DMAXSAD */
    case 0: dma[id].src_addr = (dma[id].src_addr & 0xFFFFFF00) | (value<<0 ); break;
    case 1: dma[id].src_addr = (dma[id].src_addr & 0xFFFF00FF) | (value<<8 ); break;
    case 2: dma[id].src_addr = (dma[id].src_addr & 0xFF00FFFF) | (value<<16); break;
    case 3: dma[id].src_addr = (dma[id].src_addr & 0x00FFFFFF) | (value<<24); break;

    /* DMAXDAD */
    case 4: dma[id].dst_addr = (dma[id].dst_addr & 0xFFFFFF00) | (value<<0 ); break;
    case 5: dma[id].dst_addr = (dma[id].dst_addr & 0xFFFF00FF) | (value<<8 ); break;
    case 6: dma[id].dst_addr = (dma[id].dst_addr & 0xFF00FFFF) | (value<<16); break;
    case 7: dma[id].dst_addr = (dma[id].dst_addr & 0x00FFFFFF) | (value<<24); break;

    /* DMAXCNT_L */
    case 8: dma[id].length = (dma[id].length & 0xFF00) | (value<<0); break;
    case 9: dma[id].length = (dma[id].length & 0x00FF) | (value<<8); break;

    /* DMAXCNT_H */
    case 10: {
      dma[id].dst_cntl = AddressControl((value >> 5) & 3);
      dma[id].src_cntl = AddressControl((dma[id].src_cntl & 0b10) | (value>>7));
      break;
    }
    case 11: {
      bool enable_previous = dma[id].enable;

      dma[id].src_cntl  = AddressControl((dma[id].src_cntl & 0b01) | ((value & 1)<<1));
      dma[id].size      = WordSize((value>>2) & 1);
      dma[id].time      = StartTiming((value>>4) & 3);
      dma[id].repeat    = value & 2;
      dma[id].gamepak   = value & 8;
      dma[id].interrupt = value & 64;
      dma[id].enable    = value & 128;

      hblank_set &= ~(1<<id);
      vblank_set &= ~(1<<id);

      if (dma[id].enable) {
        /* Update HBLANK/VBLANK DMA sets. */
        switch (dma[id].time) {
          case DMA_HBLANK:
            hblank_set |= (1<<id);
            break;
          case DMA_VBLANK:
            vblank_set |= (1<<id);
            break;
        }

        /* DMA state is latched on "rising" enable bit. */
        if (!enable_previous) {
          dma[id].internal.request_count = 0;

          /* Latch sanitized values into internal DMA state. */
          dma[id].internal.dst_addr = dma[id].dst_addr & g_dma_dst_mask[id];
          dma[id].internal.src_addr = dma[id].src_addr & g_dma_src_mask[id];
          dma[id].internal.length   = dma[id].length   & g_dma_len_mask[id];

          if (dma[id].internal.length == 0) {
            dma[id].internal.length = g_dma_len_mask[id] + 1;
          }

          /* Schedule DMA if is setup for immediate execution. */
          if (dma[id].time == DMA_IMMEDIATE) {
            MarkDMAForExecution(id);
          }
        }
      }
      break;
    }
  }
}
