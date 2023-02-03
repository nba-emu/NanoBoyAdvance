/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

static u32 RGB555(u16 rgb555) {
  const uint r = (rgb555 >>  0) & 31U;
  const uint g = (rgb555 >>  5) & 31U;
  const uint b = (rgb555 >> 10) & 31U;

  return 0xFF000000 | r << 19 | g << 11 | b << 3;
}

void PPU::InitMerge() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  merge.timestamp_init = timestamp_now;
  merge.timestamp_last_sync = timestamp_now;
  merge.cycle = 0U;
}

void PPU::DrawMerge() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  const int cycles = (int)(timestamp_now - merge.timestamp_last_sync);

  if(cycles == 0 || merge.cycle >= k_bg_cycle_limit) {
    return;
  }

  // @todo: possibly template this based on IO configuration
  DrawMergeImpl(cycles);

  merge.timestamp_last_sync = timestamp_now;
}

void PPU::DrawMergeImpl(int cycles) {
  // fmt::print("PPU: draw scanline @ VCOUNT={} delta={}\n", mmio.vcount, cycles);

  // @todo: fix numbers for Mode 6 and Mode 7
  static constexpr int k_min_max_bg[8][2] {
    {0,  3}, // Mode 0 (BG0 - BG3 text-mode)
    {0,  2}, // Mode 1 (BG0 - BG1 text-mode, BG2 affine)
    {2,  3}, // Mode 2 (BG2 - BG3 affine)
    {2,  2}, // Mode 3 (BG2 240x160 65526-color bitmap)
    {2,  2}, // Mode 4 (BG2 240x160 256-color bitmap, double-buffered)
    {2,  2}, // Mode 5 (BG2 160x128 65536-color bitmap, double-buffered)
    {0, -1}, // Mode 6 (invalid)
    {0, -1}, // Mode 7 (invalid)
  };

  const int mode = mmio.dispcnt.mode;

  const int min_bg = k_min_max_bg[mode][0];
  const int max_bg = k_min_max_bg[mode][1];

  // Enabled BGs sorted from lowest to highest priority.
  int bg_list[4];
  int bg_count = 0;

  for(int priority = 3; priority >= 0; priority--) {
    for(int id = max_bg; id >= min_bg; id--) {
      if(mmio.bgcnt[id].priority == priority &&
         mmio.enable_bg[0][id] &&
         mmio.dispcnt.enable[id]) {
        bg_list[bg_count++] = id;
      }
    }
  }

  for(int i = 0; i < cycles; i++) {
    const int cycle = (int)merge.cycle - 46;

    if(cycle >= 0 && (cycle & 3) == 0) {
      const uint x = (uint)cycle >> 2;

      uint final_color = 0U;
      uint priority = 3U;

      for(int i = 0; i < bg_count; i++) {
        const int id = bg_list[i];
        const uint color = bg.buffer[x][id];

        if(color != 0U) {
          final_color = color;
          priority = (uint)mmio.bgcnt[id].priority;
        }
      }

      if(mmio.dispcnt.enable[LAYER_OBJ]) {
        auto& pixel = sprite.buffer_rd[x];

        if(pixel.color != 0U && pixel.priority <= priority) {
          final_color = (uint)pixel.color | 256U;
        }
      }

      output[frame][mmio.vcount * 240 + x] = RGB555(FetchPRAM<u16>(merge.cycle, final_color << 1));
    }

    if(++merge.cycle == k_bg_cycle_limit) {
      break;
    }
  }
}

} // namespace nba::core
