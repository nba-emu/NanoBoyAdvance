
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
      u32 pixel = bg[2].buffer[8 + x];

      switch (pixel & 0xC000'0000) {
        case 0x0000'0000: *buffer++ = RGB565(read<u16>(pram, pixel << 1)); break;
        case 0x4000'0000: *buffer++ = RGB565(pixel & 0xFFFF); break;
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
