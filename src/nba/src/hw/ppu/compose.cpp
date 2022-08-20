
#include "hw/ppu/ppu.hpp"

namespace nba::core {

// TODO: move this somewhere sane
const auto RGB565 = [](u16 rgb565) -> u32 {
  int r = (rgb565 >>  0) & 31;
  int g = (rgb565 >>  5) & 31;
  int b = (rgb565 >> 10) & 31;

  // TODO: handle the 6-th green channel bit
  // TODO: fill the lower bits of each channel

  return (r << 19) |
         (g << 11) |
         (b <<  3) | 0xFF000000;
};

void PPU::InitCompose() {
  compose.engaged = true;
  compose.hcounter = 0;
}

void PPU::SyncCompose(int cycles) {
  const int RENDER_DELAY = 41; // TODO: figure out the exact value.

  int hcounter = compose.hcounter;
  int hcounter_target = hcounter + cycles;

  compose.hcounter = hcounter_target;

  hcounter = std::max(hcounter, RENDER_DELAY);

  int x = (hcounter - RENDER_DELAY) >> 2;
  u16 backdrop = read<u16>(pram, 0);
  u32* buffer = &output[frame][mmio.vcount * 240 + x];

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    if (cycle == 0) {
      int prio[2] { 4, 4 };
      int layer[2] { LAYER_BD, LAYER_BD };
      u32 color[2] { 0x8000'0000, 0x8000'0000 };

      // Find up to two top-most visible background pixels.
      // TODO: is the BG enable logic correct?
      // And can we optimize this algorithm more without sacrificing accuracy?
      for (int priority = 3; priority >= 0; priority--) {
        for (int id = 3; id >= 0; id--) {
          if (mmio.enable_bg[0][id] && mmio.dispcnt.enable[id] && mmio.bgcnt[id].priority == priority && bg[id].buffer[8 + x] != 0x8000'0000) {
            prio[1] = prio[0];
            prio[0] = priority;
            layer[1] = layer[0];
            layer[0] = id;
            color[1] = color[0];
            color[0] = bg[id].buffer[8 + x];
          }
        }
      }

      /* Check if a OBJ pixel takes priority over one of the two
       * top-most background pixels and insert it accordingly.
       */
      // TODO: is the OBJ enable bit delayed like for BGs?
      if (mmio.dispcnt.enable[LAYER_OBJ]) {
        auto pixel = buffer_obj[x];
        
        if (pixel.color != 0x8000'0000) {
          if (pixel.priority <= prio[0]) {
            prio[1] = prio[0];
            prio[0] = pixel.priority;
            layer[1] = layer[0];
            layer[0] = LAYER_OBJ;
            color[1] = color[0];
            color[0] = pixel.color;
          } else if (pixel.priority <= prio[1]) {
            prio[1] = pixel.priority;
            layer[1] = LAYER_OBJ;
            color[1] = pixel.color;
          }
        }
      } 

      // TODO: blending
      switch (color[0] & 0xC000'0000) {
        case 0x0000'0000: *buffer++ = RGB565(read<u16>(pram, color[0] << 1)); break;
        case 0x4000'0000: *buffer++ = RGB565(color[0] & 0xFFFF); break;
        case 0x8000'0000: *buffer++ = RGB565(backdrop); break;
      }

      hcounter += 4;

      if (++x == 240) {
        compose.engaged = false;
        break;
      }
    } else {
      hcounter += 4 - cycle;
    }
  }
}

} // namespace nba::core
