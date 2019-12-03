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
#include "memory/registers.hpp"

using namespace GameBoyAdvance;

/* TODO: what happens if src_cntl equals DMA::RELOAD? */
static constexpr int g_dma_modify[2][4] = {
  { 2, -2, 0, 2 },
  { 4, -4, 0, 4 }
};

static constexpr int g_dma_none_id = -1;

/* Retrieves DMA with highest priority from a DMA bitset. */
static constexpr int g_dma_from_bitset[] = {
  /* 0b0000 */ g_dma_none_id,
  /* 0b0001 */ 0,
  /* 0b0010 */ 1,
  /* 0b0011 */ 0,
  /* 0b0100 */ 2,
  /* 0b0101 */ 0,
  /* 0b0110 */ 1,
  /* 0b0111 */ 0,
  /* 0b1000 */ 3,
  /* 0b1001 */ 0,
  /* 0b1010 */ 1,
  /* 0b1011 */ 0,
  /* 0b1100 */ 2,
  /* 0b1101 */ 0,
  /* 0b1110 */ 1,
  /* 0b1111 */ 0
};

static constexpr std::uint32_t g_dma_len_mask[] = { 
  0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF
};

void DMA::Reset() {
  hblank_set.reset();
  vblank_set.reset();
  video_set.reset();
  runnable.reset();
  
  current = -1;
  interleaved = false;
  
  for (auto& channel : channels) {
    channel.enable = false;
    channel.repeat = false;
    channel.interrupt = false;
    channel.gamepak = false;
    channel.length  = 0;
    channel.dst_addr = 0;
    channel.src_addr = 0;
    channel.latch.length = 0;
    channel.latch.dst_addr = 0;
    channel.latch.src_addr = 0;
    channel.size = Channel::HWORD;
    channel.time = Channel::IMMEDIATE;
    channel.dst_cntl = Channel::INCREMENT;
    channel.src_cntl = Channel::INCREMENT;
  }
}

void DMA::TryStart(int chan_id) {
  if (runnable.none()) {
    current = chan_id;
  } else if (chan_id < current) {
    current = chan_id;
    interleaved = true;
  }
  
  runnable.set(chan_id, true);
}

void DMA::Request(Occasion occasion) {
  switch (occasion) {
    case Occasion::HBlank: {
      auto chan_id = g_dma_from_bitset[hblank_set.to_ulong()];
      if (chan_id != g_dma_none_id) {
        TryStart(chan_id);
        runnable |= hblank_set;
      }
      break;
    }
    case Occasion::VBlank: {
      auto chan_id = g_dma_from_bitset[vblank_set.to_ulong()];
      if (chan_id != g_dma_none_id) {
        TryStart(chan_id);
        runnable |= vblank_set;
      }
      break;
    }
    case Occasion::Video: {
      auto chan_id = g_dma_from_bitset[video_set.to_ulong()];
      if (chan_id != g_dma_none_id) {
        TryStart(chan_id);
        runnable |= video_set;
      }
      break;
    }
    case Occasion::FIFO0:
    case Occasion::FIFO1: {
      auto address = (occasion == Occasion::FIFO0) ? FIFO_A : FIFO_B;
      for (int chan_id = 1; chan_id <= 2; chan_id++) {
        auto& channel = channels[chan_id];
        if (channel.enable &&
            channel.time == Channel::SPECIAL &&
            channel.dst_addr == address) {
          channel.fifo_request_count++;
          TryStart(chan_id);
        }
      }
      break;
    }
  }
}

void DMA::Run(int const& ticks_left) {
  auto& channel = channels[current];
  
  auto access = ARM::ACCESS_NSEQ;
  
  if (channel.is_fifo_dma) {
    /* TODO: figure out how the FIFO DMA works in detail. */
    for (int i = 0; i < 4; i++) {
      if (channel.allow_read) {
        latch = cpu->ReadWord(channel.latch.src_addr, access);
      }
      cpu->WriteWord(channel.latch.dst_addr, latch, access);
      access = channel.second_access;
      channel.latch.src_addr += 4;
    }

    if (--channel.fifo_request_count == 0) {
      runnable.set(current, false);
    }
  } else {
    auto src_modify = g_dma_modify[channel.size][channel.src_cntl];
    auto dst_modify = g_dma_modify[channel.size][channel.dst_cntl];
    
    #define CHECK_INTERLEAVED\
      if (ticks_left <= 0 || interleaved) {\
        interleaved = false;\
        return;\
      }
    
    #define ADVANCE_REGS\
      channel.latch.src_addr += src_modify;\
      channel.latch.dst_addr += dst_modify;\
      channel.latch.length--;
    
    if (channel.size == Channel::HWORD) {
      if (channel.allow_read) {
        /* HWORD DMA - NORMAL */
        while (channel.latch.length != 0) {
          CHECK_INTERLEAVED;

          latch = 0x00010001 * cpu->ReadHalf(channel.latch.src_addr, access);
          cpu->WriteHalf(channel.latch.dst_addr, latch, access);
          access = channel.second_access;
          ADVANCE_REGS;
        }
      } else {
        /* HWORD DMA - OPEN BUS */
        while (channel.latch.length != 0) {
          CHECK_INTERLEAVED;
          
          cpu->WriteHalf(channel.latch.dst_addr, latch, access);
          access = channel.second_access;
          ADVANCE_REGS;
        }
      }
    } else {
      if (channel.allow_read) {
        /* WORD DMA - NORMAL */
        while (channel.latch.length != 0) {
          CHECK_INTERLEAVED;
          
          latch = cpu->ReadWord(channel.latch.src_addr, access);
          cpu->WriteWord(channel.latch.dst_addr, latch, access);
          access = channel.second_access;
          ADVANCE_REGS;
        }
      } else {
        /* WORD DMA - OPEN BUS */
        while (channel.latch.length != 0) {
          CHECK_INTERLEAVED;
          
          cpu->WriteWord(channel.latch.dst_addr, latch, access);
          access = channel.second_access;
          ADVANCE_REGS;
        }
      }
    }
    
    /* If this code is reached, the DMA was not interleaved and completed. */
    if (channel.repeat) {
      /* Latch length */
      channel.latch.length = channel.length & g_dma_len_mask[current];
      if (channel.latch.length == 0) {
        channel.latch.length = g_dma_len_mask[current] + 1;
      }
      
      /* Latch destination address */
      if (channel.dst_cntl == Channel::RELOAD) {
        if (CheckDestinationAddress(current, channel.dst_addr >> 24)) {
          channel.latch.dst_addr = channel.dst_addr;
          /* Ensure that all writes are aligned. */
          if (channel.size == Channel::WORD) {
            channel.latch.dst_addr &= ~3;
          } else {
            channel.latch.dst_addr &= ~1;
          }
        } else {
          /* TODO: disable write completely? */
          channel.latch.dst_addr = 0;
        }
      }
      
      /* Non-immediate DMAs must be retriggered before they run again. */
      if (channel.time != Channel::IMMEDIATE) {
        runnable.set(current, false);
      }
    } else {
      channel.enable = false;
      
      /* DMA is not enabled, thus not eligable for HBLANK/VBLANK/VIDEO requests. */
      runnable.set(current, false);
      hblank_set.set(current, false);
      vblank_set.set(current, false);
      video_set.set(current, false);
    }
  }
  
  if (channel.interrupt) {
    cpu->mmio.irq_if |= (CPU::INT_DMA0 << current);
  }
  
  /* Select the next DMA to execute */
  current = g_dma_from_bitset[runnable.to_ulong()];
}

auto DMA::Read(int chan_id, int offset) -> std::uint8_t {
  auto const& channel = channels[chan_id];

  switch (offset) {
    case 10: {
      return (channel.dst_cntl << 5) |
             (channel.src_cntl << 7);
    }
    case 11: {
      return (channel.src_cntl  >> 1) |
             (channel.size      << 2) |
             (channel.time      << 4) |
             (channel.repeat    ? 2   : 0) |
             (channel.gamepak   ? 8   : 0) |
             (channel.interrupt ? 64  : 0) |
             (channel.enable    ? 128 : 0);
    }
    default: return 0;
  }
}

void DMA::Write(int chan_id, int offset, std::uint8_t value) {
  auto& channel = channels[chan_id];

  switch (offset) {
    /* DMAXSAD
     * NOTE: the most-significant nibble will be removed here. */
    case 0: channel.src_addr = (channel.src_addr & 0x0FFFFF00) |  (value<<0 ); break;
    case 1: channel.src_addr = (channel.src_addr & 0x0FFF00FF) |  (value<<8 ); break;
    case 2: channel.src_addr = (channel.src_addr & 0x0F00FFFF) |  (value<<16); break;
    case 3: channel.src_addr = (channel.src_addr & 0x00FFFFFF) | ((value<<24) & 0x0F000000); break;

    /* DMAXDAD
     * NOTE: the most-significant nibble will be removed here. */
    case 4: channel.dst_addr = (channel.dst_addr & 0x0FFFFF00) |  (value<<0 ); break;
    case 5: channel.dst_addr = (channel.dst_addr & 0x0FFF00FF) |  (value<<8 ); break;
    case 6: channel.dst_addr = (channel.dst_addr & 0x0F00FFFF) |  (value<<16); break;
    case 7: channel.dst_addr = (channel.dst_addr & 0x00FFFFFF) | ((value<<24) & 0x0F000000); break;

    /* DMAXCNT_L */
    case 8: channel.length = (channel.length & 0xFF00) | (value<<0); break;
    case 9: channel.length = (channel.length & 0x00FF) | (value<<8); break;

    /* DMAXCNT_H */
    case 10: {
      channel.dst_cntl = Channel::Control((value >> 5) & 3);
      channel.src_cntl = Channel::Control((channel.src_cntl & 0b10) | (value>>7));
      break;
    }
    case 11: {
      bool enable_previous = channel.enable;

      channel.src_cntl  = Channel::Control((channel.src_cntl & 0b01) | ((value & 1)<<1));
      channel.size      = Channel::Size((value>>2) & 1);
      channel.time      = Channel::Timing((value>>4) & 3);
      channel.repeat    =  value & 2;
      channel.gamepak   = (value & 8) && chan_id == 3;
      channel.interrupt =  value & 64;
      channel.enable    =  value & 128;
      
      channel.is_fifo_dma = (channel.time == Channel::SPECIAL) && 
                            (chan_id == 1 || chan_id == 2);
      
      OnChannelWritten(chan_id, enable_previous);
      break;
    }
  }
}

void DMA::OnChannelWritten(int chan_id, bool enabled_old) {
  auto& channel = channels[chan_id];

  if (channel.enable) {
    /* Update HBLANK/VBLANK DMA sets. 
     * This information is used to schedule these DMAs on request.
     */
    switch (channel.time) {
      case Channel::HBLANK:
        hblank_set.set(chan_id, true);
        break;
      case Channel::VBLANK:
        vblank_set.set(chan_id, true);
        break;
      case Channel::SPECIAL:
        if (chan_id == 3) {
          video_set.set(chan_id, true);
        }
        break;
    }

    /* DMA state is latched on "rising" enable bit. */
    if (!enabled_old) {
      int src_page = GetUnaliasedMemoryArea(channel.src_addr >> 24);
      int dst_page = GetUnaliasedMemoryArea(channel.dst_addr >> 24);
      
      /* Latch destination address */
      if (CheckDestinationAddress(chan_id, dst_page)) {
        channel.latch.dst_addr = channel.dst_addr;
      } else {
        /* TODO: disable write completely? */
        channel.latch.dst_addr = 0;
      }
      
      /* Latch source address */
      if (CheckSourceAddress(chan_id, src_page)) {
        channel.latch.src_addr = channel.src_addr;
        channel.allow_read = true;
      } else {
        channel.allow_read = false;
      }
      
      /* Ensure that all accesses are aligned. */
      if (channel.size == Channel::WORD) {
        channel.latch.src_addr &= ~3;
        channel.latch.dst_addr &= ~3;
      } else {
        channel.latch.src_addr &= ~1;
        channel.latch.dst_addr &= ~1;
      }
      
      /* Latch length */
      channel.latch.length = channel.length & g_dma_len_mask[chan_id];
      if (channel.latch.length == 0) {
        channel.latch.length = g_dma_len_mask[chan_id] + 1;
      }
      
      /* TODO: confirm that read/write to the same area results 
       * in only non-sequential accesses.
       */
      if (src_page == dst_page) {
        channel.second_access = ARM::ACCESS_NSEQ;
      } else {
        channel.second_access = ARM::ACCESS_SEQ;
      }
      
      /* Immediate DMAs are scheduled right away. */
      if (channel.time == Channel::IMMEDIATE) {
        TryStart(chan_id);
      }
      
      channel.fifo_request_count = 0;
    }
  } else {
    /* DMA is not enabled, thus not eligable for HBLANK/VBLANK/VIDEO requests. */
    hblank_set.set(chan_id, false);
    vblank_set.set(chan_id, false);
    video_set.set(chan_id, false);
  }
}
