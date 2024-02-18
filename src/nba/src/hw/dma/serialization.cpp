/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/dma/dma.hpp"

namespace nba::core {

void DMA::LoadState(SaveState const& state) {
  should_reenter_transfer_loop = false;

  hblank_set = state.dma.hblank_set;
  vblank_set = state.dma.vblank_set;
  video_set = state.dma.video_set;
  runnable_set = state.dma.runnable_set;
  latch = state.dma.latch;

  for(int i = 0; i < 4; i++) {
    auto& channel_src = state.dma.channels[i];
    auto& channel_dst = channels[i];

    u16 control = channel_src.control;

    channel_dst.dst_addr = channel_src.dst_address;
    channel_dst.src_addr = channel_src.src_address;
    channel_dst.length = channel_src.length;

    channel_dst.enable = control & 32768;
    channel_dst.repeat = control & 512;
    channel_dst.interrupt = control & 16384;
    channel_dst.gamepak = control & 2048;
    channel_dst.dst_cntl = (Channel::Control)((control >> 5) & 3);
    channel_dst.src_cntl = (Channel::Control)((control >> 7) & 3);
    channel_dst.time = (Channel::Timing)((control >> 12) & 3);
    channel_dst.size = (Channel::Size)((control >> 10) & 1);

    channel_dst.latch.dst_addr = channel_src.latch.dst_address;
    channel_dst.latch.src_addr = channel_src.latch.src_address;
    channel_dst.latch.length = channel_src.latch.length;
    channel_dst.latch.bus = channel_src.latch.bus;

    channel_dst.is_fifo_dma = channel_src.is_fifo_dma;

    channel_dst.event = scheduler.GetEventByUID(channel_src.event_uid);
  }

  SelectNextDMA();
}

void DMA::CopyState(SaveState& state) {
  state.dma.hblank_set = (u8)hblank_set.to_ulong();
  state.dma.vblank_set = (u8)vblank_set.to_ulong();
  state.dma.video_set = (u8)video_set.to_ulong();
  state.dma.runnable_set = (u8)runnable_set.to_ulong();
  state.dma.latch = latch;

  for(int i = 0; i < 4; i++) {
    auto& channel_src = channels[i];
    auto& channel_dst = state.dma.channels[i];

    channel_dst.dst_address = channel_src.dst_addr;
    channel_dst.src_address = channel_src.src_addr;
    channel_dst.length = channel_src.length;

    channel_dst.control = ((int)channel_src.dst_cntl << 5) |
                          ((int)channel_src.src_cntl << 7) |
                          (channel_src.repeat ? 512 : 0) |
                          ((int)channel_src.size << 10) |
                          (channel_src.gamepak ? 2048 : 0) |
                          ((int)channel_src.time << 12) |
                          (channel_src.interrupt ? 16384 : 0) |
                          (channel_src.enable ? 32768 : 0);

    channel_dst.latch.dst_address = channel_src.latch.dst_addr;
    channel_dst.latch.src_address = channel_src.latch.src_addr;
    channel_dst.latch.length = channel_src.latch.length;
    channel_dst.latch.bus = channel_src.latch.bus;

    channel_dst.is_fifo_dma = channel_src.is_fifo_dma;

    channel_dst.event_uid = GetEventUID(channel_src.event);
  }
}

} // namespace nba::core