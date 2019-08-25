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

#include "../cpu.hpp"
#include "../memory/mmio.hpp"

namespace GameBoyAdvance {

/* TODO: what happens if src_cntl equals DMA::RELOAD? */
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

constexpr std::uint32_t CPU::s_dma_dst_mask[4];
constexpr std::uint32_t CPU::s_dma_src_mask[4];
constexpr std::uint32_t CPU::s_dma_len_mask[4];

void CPU::StartDMA(int id) {
  /* If no other DMA is running or this DMA has higher priority
   * then execute this DMA directly.
   * Lower priority DMAs will be interleaved in the latter case.
   */
  if (dma_run_set == 0) {
    dma_current = id;
  } else if (id < dma_current) {
    dma_current = id;
    dma_interleaved = true;
  }

  dma_run_set |= (1 << id);
}

void CPU::RequestHBlankDMA() {
  int hblank_dma = g_dma_from_bitset[dma_hblank_set];
  
  if (hblank_dma >= 0) {
    StartDMA(hblank_dma);
  }
}

void CPU::RequestVBlankDMA() {
  int vblank_dma = g_dma_from_bitset[dma_vblank_set];
  
  if (vblank_dma >= 0) {
    StartDMA(vblank_dma);
  }
}

void CPU::RequestAudioDMA(int fifo) {
  std::uint32_t address[2] = { FIFO_A, FIFO_B };
  
  for (int id = 1; id <= 2; id++) {
    if (mmio.dma[id].enable &&
        mmio.dma[id].time == DMA::SPECIAL &&
        mmio.dma[id].dst_addr == address[fifo])
    {
      mmio.dma[id].internal.request_count++;
      StartDMA(id);
      break;
    }
  }
}

void CPU::RunDMA() {
  auto& dma = mmio.dma[dma_current];
  
  auto src_modify = g_dma_modify[dma.size][dma.src_cntl];
  auto dst_modify = g_dma_modify[dma.size][dma.dst_cntl];
  
  /* FIFO DMA does not work quite a like a normal DMA.
   * For example the repeat bit works different, word transfer is enforced
   * and if FIFO DMA was requested multiple times, it needs to run more than once.
   */
  if (dma.time == DMA::SPECIAL && (dma_current == 1 || dma_current == 2)) {
    RunAudioDMA();
    return;
  }
  
  std::uint32_t word;
  
  /* Run DMA until completion or interruption. */
  if (dma.size == DMA::WORD) {
    while (dma.internal.length != 0) {
      if (ticks_cpu_left <= 0) return;

      /* Stop if DMA was interleaved by higher priority DMA. */
      if (dma_interleaved) {
        dma_interleaved = false;
        return;
      }

      word = ReadWord(dma.internal.src_addr & ~3, ARM::ACCESS_SEQ);
      WriteWord(dma.internal.dst_addr & ~3, word, ARM::ACCESS_SEQ);

      dma.internal.src_addr += src_modify;
      dma.internal.dst_addr += dst_modify;
      dma.internal.length--;
    }
  } else {
    while (dma.internal.length != 0) {
      if (ticks_cpu_left <= 0) return;

      /* Stop if DMA was interleaved by higher priority DMA. */
      if (dma_interleaved) {
        dma_interleaved = false;
        return;
      }

      word = ReadHalf(dma.internal.src_addr & ~1, ARM::ACCESS_SEQ);
      WriteHalf(dma.internal.dst_addr & ~1, word, ARM::ACCESS_SEQ);
      
      dma.internal.src_addr += src_modify;
      dma.internal.dst_addr += dst_modify;
      dma.internal.length--;
    }
  }

  
  if (dma.interrupt) {
    mmio.irq_if |= CPU::INT_DMA0 << dma_current;
  }
  
  if (dma.repeat) {
    /* Reload the internal length counter. */
    dma.internal.length = dma.length & s_dma_len_mask[dma_current];
    if (dma.internal.length == 0) {
      dma.internal.length = s_dma_len_mask[dma_current] + 1;
    }

    /* Reload destination address if specified. */
    if (dma.dst_cntl == DMA::RELOAD) {
      dma.internal.dst_addr = dma.dst_addr & s_dma_dst_mask[dma_current];
    }

    /* If DMA is specified to be non-immediate, wait for it to be retriggered. */
    if (dma.time != DMA::IMMEDIATE) {
      dma_run_set &= ~(1 << dma_current);
    }
  } else {
    dma.enable = false;
    dma_run_set &= ~(1 << dma_current);
  }
  
  dma_current = g_dma_from_bitset[dma_run_set];
}

void CPU::RunAudioDMA() {
  auto& dma = mmio.dma[dma_current];
  
  std::uint32_t word;
  
  for (int i = 0; i < 4; i++) {
    word = ReadWord(dma.internal.src_addr & ~3, ARM::ACCESS_SEQ);
    WriteWord(dma.internal.dst_addr & ~3, word, ARM::ACCESS_SEQ);
    dma.internal.src_addr += 4;
  }
    
  if (--dma.internal.request_count == 0) {
    dma_run_set &= ~(1 << dma_current);
  }
    
  if (dma.interrupt) {
    mmio.irq_if |= CPU::INT_DMA0 << dma_current;
  }
    
  dma_current = g_dma_from_bitset[dma_run_set];
}

}