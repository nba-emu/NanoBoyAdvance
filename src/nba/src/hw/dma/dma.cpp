/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/common/compiler.hpp>

#include "bus/bus.hpp"
#include "bus/io.hpp"
#include "hw/dma/dma.hpp"

namespace nba::core {

static constexpr int g_dma_src_modify[2][4] = {
  { 2, -2, 0, 0 },
  { 4, -4, 0, 0 }
};

static constexpr int g_dma_dst_modify[2][4] = {
  { 2, -2, 0, 2 },
  { 4, -4, 0, 4 }
};

static constexpr int g_dma_none_id = -1;

// Get DMA with highest priority from a bitset.
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

static constexpr u32 g_dma_src_mask[] = {
  0x07FFFFFF, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF
};

static constexpr u32 g_dma_dst_mask[] = {
  0x07FFFFFF, 0x07FFFFFF, 0x07FFFFFF, 0x0FFFFFFF
};

static constexpr u32 g_dma_len_mask[] = {
  0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF
};

static constexpr char const* g_address_mode_name[] = {
  "+", "-", "C", "R+"
};

static constexpr char const* g_dma_time_name[] = {
  "imm",
  "vblank",
  "hblank",
  "audio"
};

DMA::DMA(Bus& bus, IRQ& irq, Scheduler& scheduler)
    : bus(bus)
    , irq(irq)
    , scheduler(scheduler) {
  scheduler.Register(Scheduler::EventClass::DMA_activated, this, &DMA::OnActivated);

  Reset();
}

void DMA::Reset() {
  active_dma_id = g_dma_none_id;
  should_reenter_transfer_loop = false;
  hblank_set = 0;
  vblank_set = 0;
  video_set = 0;
  runnable_set = 0;

  for(int id = 0; id < 4; id++) {
    channels[id] = {};
    channels[id].id = id;
  }
}

void DMA::ScheduleDMAs(unsigned int bitset) {
  while(bitset != 0) {
    auto chan_id = g_dma_from_bitset[bitset];

    bitset &= ~(1 << chan_id);

    channels[chan_id].event = scheduler.Add(2, Scheduler::EventClass::DMA_activated, 0, chan_id);
  }
}

void DMA::OnActivated(u64 chan_id) {
  channels[chan_id].event = nullptr;

  if(runnable_set.none()) {
    active_dma_id = chan_id;
  } else if(chan_id < active_dma_id) {
    active_dma_id = chan_id;
    should_reenter_transfer_loop = true;
  }

  runnable_set.set(chan_id, true);
}

void DMA::SelectNextDMA() {
  active_dma_id = g_dma_from_bitset[runnable_set.to_ulong()];
}

void DMA::Request(Occasion occasion) {
  switch(occasion) {
    case Occasion::HBlank:
      ScheduleDMAs(hblank_set.to_ulong());
      break;
    case Occasion::VBlank:
      ScheduleDMAs(vblank_set.to_ulong());
      break;
    case Occasion::Video:
      ScheduleDMAs(video_set.to_ulong());
      break;
    case Occasion::FIFO0: 
      if(channels[1].enable && channels[1].time == Channel::Special) ScheduleDMAs(2);
      break;
    case Occasion::FIFO1: 
      if(channels[2].enable && channels[2].time == Channel::Special) ScheduleDMAs(4);
      break;
  }
}

void DMA::StopVideoTransferDMA() {
  auto& channel = channels[3];

  // Disable DMA3
  // TODO: does HW disable it regardless of the *current* configuration?
  if(channel.enable) {
    channel.enable = false;
    OnChannelWritten(channel, true);
  }
}

bool DMA::HasVideoTransferDMA() {
  return channels[3].enable &&
         channels[3].time == Channel::Timing::Special;
}

auto DMA::Run() -> int {
  const auto timestamp0 = scheduler.GetTimestampNow();

  bus.Step(1);

  do {
    RunChannel();
  } while(IsRunning());

  bus.Step(1);

  return (int)(scheduler.GetTimestampNow() - timestamp0);
}

void DMA::RunChannel() {
  auto& channel = channels[active_dma_id];
  int dst_modify;
  int src_modify;
  auto size = channel.size;

  if(channel.is_fifo_dma) {
    size = Channel::Size::Word;
    dst_modify = 0;
  } else {
    dst_modify = g_dma_dst_modify[size][channel.dst_cntl];
  }
  src_modify = g_dma_src_modify[size][channel.src_cntl];

  bool did_access_rom = false;

  while(channel.latch.length != 0) {
    if(should_reenter_transfer_loop) {
      should_reenter_transfer_loop = false;
      return;
    }

    auto src_addr = channel.latch.src_addr;
    auto dst_addr = channel.latch.dst_addr;

    auto access_src = Bus::Access::Sequential | Bus::Access::Dma;
    auto access_dst = Bus::Access::Sequential | Bus::Access::Dma;

    if(!did_access_rom) {
      if(src_addr >= 0x08000000) {
        access_src = Bus::Access::Nonsequential | Bus::Access::Dma;
        did_access_rom = true;
      } else if(dst_addr >= 0x08000000) {
        access_dst = Bus::Access::Nonsequential | Bus::Access::Dma;
        did_access_rom = true;
      }
    }

    if(size == Channel::Half) {
      u16 value;

      if(likely(src_addr >= 0x02000000)) {
        value = bus.ReadHalf(src_addr, access_src);
        channel.latch.bus = (value << 16) | value;
        latch = channel.latch.bus;
      } else {
        if(dst_addr & 2) {
          value = channel.latch.bus >> 16;
        } else {
          value = channel.latch.bus;
        }
        bus.Step(1);
      }

      bus.WriteHalf(dst_addr, value, access_dst);
    } else {
      if(likely(src_addr >= 0x02000000)) {
        channel.latch.bus = bus.ReadWord(src_addr, access_src);
        latch = channel.latch.bus;
      } else {
        bus.Step(1);
      }

      bus.WriteWord(dst_addr, channel.latch.bus, access_dst);
    }

    channel.latch.src_addr += src_modify;
    channel.latch.dst_addr += dst_modify;
    channel.latch.length--;
  }

  runnable_set.set(channel.id, false);

  if(channel.interrupt) {
    irq.Raise(IRQ::Source::DMA, channel.id);
  }

  if(channel.repeat && channel.time != Channel::Immediate) {
    if(channel.is_fifo_dma) {
      channel.latch.length = 4;
    } else {
      channel.latch.length = channel.length & g_dma_len_mask[channel.id];
      if(channel.latch.length == 0) {
        channel.latch.length = g_dma_len_mask[channel.id] + 1;
      }
    }

    if(channel.dst_cntl == Channel::Reload && !channel.is_fifo_dma) {
      auto mask = channel.size == Channel::Word ? ~3 : ~1;
      channel.latch.dst_addr = channel.dst_addr & mask;
    }
  } else {
    RemoveChannelFromDMASets(channel);
    channel.enable = false;
  }

  SelectNextDMA();
}

auto DMA::Read(int chan_id, int offset) -> u8 {
  auto const& channel = channels[chan_id];

  switch(offset) {
    case REG_DMAXCNT_H | 0: {
      return (channel.dst_cntl << 5) |
             (channel.src_cntl << 7);
    }
    case REG_DMAXCNT_H | 1: {
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

void DMA::Write(int chan_id, int offset, u8 value) {
  auto& channel = channels[chan_id];

  switch(offset) {
    case REG_DMAXSAD | 0:
    case REG_DMAXSAD | 1:
    case REG_DMAXSAD | 2:
    case REG_DMAXSAD | 3: {
      int shift = offset * 8;
      channel.src_addr &= ~(0xFFUL << shift);
      channel.src_addr |= (value << shift) & g_dma_src_mask[chan_id];
      break;
    }
    case REG_DMAXDAD | 0:
    case REG_DMAXDAD | 1:
    case REG_DMAXDAD | 2:
    case REG_DMAXDAD | 3: {
      int shift = (offset - 4) * 8;
      channel.dst_addr &= ~(0xFFUL << shift);
      channel.dst_addr |= (value << shift) & g_dma_dst_mask[chan_id];
      break;
    }
    case REG_DMAXCNT_L | 0: channel.length = (channel.length & 0xFF00) | (value << 0); break;
    case REG_DMAXCNT_L | 1: channel.length = (channel.length & 0x00FF) | (value << 8); break;
    case REG_DMAXCNT_H | 0: {
      channel.dst_cntl = static_cast<Channel::Control>((value >> 5) & 3);
      channel.src_cntl = static_cast<Channel::Control>((channel.src_cntl & 0b10) | (value>>7));
      break;
    }
    case REG_DMAXCNT_H | 1: {
      bool enable_old = channel.enable;

      channel.src_cntl  = Channel::Control((channel.src_cntl & 0b01) | ((value & 1) << 1));
      channel.size = static_cast<Channel::Size>((value >> 2) & 1);
      channel.time = static_cast<Channel::Timing>((value >> 4) & 3);
      channel.repeat  =  value & 2;
      channel.gamepak = (value & 8) && chan_id == 3;
      channel.interrupt = value & 64;
      channel.enable = value & 128;

      OnChannelWritten(channel, enable_old);
      break;
    }
  }
}

void DMA::OnChannelWritten(Channel& channel, bool enable_old) {
  bool enable_new = channel.enable;

  RemoveChannelFromDMASets(channel);

  if(enable_new) {
    if(!enable_old) {
      // DMA enable bit: 0 -> 1 (rising-edge)
      int src_page = GetUnaliasedMemoryArea(channel.src_addr >> 24);

      channel.latch.dst_addr = channel.dst_addr;
      channel.latch.src_addr = channel.src_addr;

      if(channel.time == Channel::Special && (channel.id == 1 || channel.id == 2)) {
        channel.is_fifo_dma = true;

        channel.size = Channel::Size::Word;
        channel.latch.length = 4;
        channel.latch.src_addr &= ~3;
        channel.latch.dst_addr &= ~3;
      } else {
        channel.is_fifo_dma = false;

        auto mask = channel.size == Channel::Word ? ~3 : ~1;
        channel.latch.src_addr &= mask;
        channel.latch.dst_addr &= mask;
        channel.latch.length = channel.length & g_dma_len_mask[channel.id];
        if(channel.latch.length == 0) {
          channel.latch.length = g_dma_len_mask[channel.id] + 1;
        }

        if(channel.time == Channel::Immediate) {
          ScheduleDMAs(1 << channel.id);
        } else {
          AddChannelToDMASet(channel);
        }

        /* Try to auto-detect EEPROM size from the first EEPROM DMA transfer,
         * since we cannot always determine the size at load time.
         */
        if(channel.dst_addr >= 0x0D000000) {
          int length = channel.length;

          if(length == 9 || length == 73) {
            bus.memory.rom.SetEEPROMSizeHint(EEPROM::Size::SIZE_4K);
          }

          if(length == 17 || length == 81) {
            bus.memory.rom.SetEEPROMSizeHint(EEPROM::Size::SIZE_64K);
          }
        }
      }
    } else {
      // DMA enable bit: 1 -> 1 (remains set)

      // Note: the DMA isn't really running, while it is still enabling.
      if(channel.event == nullptr) {
        AddChannelToDMASet(channel);

        /* When the config register of a DMA is written while it is running,
         * bail out of the DMA transfer loop so that the new config is used
         * for the (half-)word transfers following this write.
         */
        if(channel.id == active_dma_id) {
          should_reenter_transfer_loop = true;
        }
      }
    }
  } else {
    // DMA enable bit: 1 -> 0 or 0 -> 0 (falling-edge or remains cleared)
    runnable_set.set(channel.id, false);

    // Handle disabling the DMA before it started up.
    if(channel.event != nullptr) {
      // TODO: figure out exact hardware behaviour.
      scheduler.Cancel(channel.event);
      channel.event = nullptr;

      Log<Warn>("DMA: disabled DMA{0} while it was starting.", channel.id);
    }

    // Handle DMA channel self-disable (via writing to its control register).
    if(channel.id == active_dma_id) {
      // TODO: figure out exact hardware behaviour.
      should_reenter_transfer_loop = true;
      SelectNextDMA();

      Log<Warn>("DMA: DMA{0} cleared its own enable bit.", channel.id);
    }
  }
}

void DMA::AddChannelToDMASet(Channel& channel) {
  switch(channel.time) {
    case Channel::HBlank: {
      hblank_set.set(channel.id, true);
      break;
    }
    case Channel::VBlank: {
      vblank_set.set(channel.id, true);
      break;
    }
    case Channel::Special: {
      if(channel.id == 3) {
        video_set.set(3, true);
      }
      break;
    }
  }
}

void DMA::RemoveChannelFromDMASets(Channel& channel) {
  hblank_set.set(channel.id, false);
  vblank_set.set(channel.id, false);
  video_set.set(channel.id, false);
}

} // namespace nba::core
