
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

  int vcount = mmio.vcount;
  int x = (hcounter - RENDER_DELAY) >> 2;
  u16 backdrop = read<u16>(pram, 0);
  u32* buffer = &output[frame][vcount * 240 + x];

  bool use_windows = mmio.dispcnt.enable[ENABLE_WIN0] ||
                     mmio.dispcnt.enable[ENABLE_WIN1] ||
                     mmio.dispcnt.enable[ENABLE_OBJWIN]; 

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    if (cycle == 0) {
      // Update windows horizontally
      // TODO: should this also happen within V-blank? Does it matter?
      for (int i = 0; i < 2; i++) {
        if (x == mmio.winh[i].min) window_flag_h[i] = true;
        if (x == mmio.winh[i].max) window_flag_h[i] = false;
      }

      const int* win_layer_enable;

      int prio[2] { 4, 4 };
      int layer[2] { LAYER_BD, LAYER_BD };
      u32 color[2] { 0x8000'0000, 0x8000'0000 };
      bool is_alpha_obj = false;

      // Find the highest-priority active window for this pixel
      if (use_windows) {
        // TODO: are the window enable bits delayed like the BG enable bits?
            if (mmio.dispcnt.enable[ENABLE_WIN0] && window_flag_v[0] && window_flag_h[0]) win_layer_enable = mmio.winin.enable[0];
        else if (mmio.dispcnt.enable[ENABLE_WIN1] && window_flag_v[1] && window_flag_h[1]) win_layer_enable = mmio.winin.enable[1];
        else if (mmio.dispcnt.enable[ENABLE_OBJ] && mmio.dispcnt.enable[ENABLE_OBJWIN] && buffer_obj[x].window) win_layer_enable = mmio.winout.enable[1];
        else win_layer_enable = mmio.winout.enable[0];
      }

      // Find up to two top-most visible background pixels.
      // TODO: is the BG enable logic correct?
      // And can we optimize this algorithm more without sacrificing accuracy?
      for (int priority = 3; priority >= 0; priority--) {
        for (int id = 3; id >= 0; id--) {
          if ((!use_windows || win_layer_enable[id]) &&
              mmio.enable_bg[0][id] &&
              mmio.dispcnt.enable[id] &&
              mmio.bgcnt[id].priority == priority &&
              bg[id].buffer[8 + x] != 0x8000'0000) {
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
      if (mmio.dispcnt.enable[LAYER_OBJ] && (!use_windows || win_layer_enable[LAYER_OBJ])) {
        auto pixel = buffer_obj[x];

        if (pixel.color != 0x8000'0000) {
          if (pixel.priority <= prio[0]) {
            prio[1] = prio[0];
            prio[0] = pixel.priority;
            layer[1] = layer[0];
            layer[0] = LAYER_OBJ;
            color[1] = color[0];
            color[0] = pixel.color;

            is_alpha_obj = pixel.alpha;
          } else if (pixel.priority <= prio[1]) {
            prio[1] = pixel.priority;
            layer[1] = LAYER_OBJ;
            color[1] = pixel.color;
          }
        }
      } 

      for (int i = 0; i < 2; i++) {
        switch (color[i] & 0xC000'0000) {
          case 0x0000'0000: color[i] = read<u16>(pram, color[i] << 1); break;
          case 0x4000'0000: color[i] &= 0xFFFF; break;
          case 0x8000'0000: color[i] = backdrop; break;
        }
      }

      bool have_blend_dst = mmio.bldcnt.targets[0][layer[0]];
      bool have_blend_src = mmio.bldcnt.targets[1][layer[1]];

      if (is_alpha_obj && have_blend_src) {
        color[0] = Blend(color[0], color[1]);
      } else if (have_blend_dst && (!use_windows || win_layer_enable[LAYER_SFX])) {
        switch (mmio.bldcnt.sfx) {
          case BlendControl::Effect::SFX_BLEND:
            if (have_blend_src) {
              color[0] = Blend(color[0], color[1]);
            }
            break;
          case BlendControl::Effect::SFX_BRIGHTEN:
            color[0] = Brighten(color[0]);
            break;
          case BlendControl::Effect::SFX_DARKEN:
            color[0] = Darken(color[0]);
            break;
        }
      }

      *buffer++ = RGB565((u16)color[0]);

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

auto PPU::Blend(u16 color_a, u16 color_b) -> u16 {
  int eva = std::min<int>(16, mmio.eva);
  int evb = std::min<int>(16, mmio.evb);

  int r_a = (color_a >>  0) & 0x1F;
  int g_a = (color_a >>  5) & 0x1F;
  int b_a = (color_a >> 10) & 0x1F;

  int r_b = (color_b >>  0) & 0x1F;
  int g_b = (color_b >>  5) & 0x1F;
  int b_b = (color_b >> 10) & 0x1F;

  int r_out = std::min<u8>((r_a * eva + r_b * evb + 8) >> 4, 31);
  int g_out = std::min<u8>((g_a * eva + g_b * evb + 8) >> 4, 31);
  int b_out = std::min<u8>((b_a * eva + b_b * evb + 8) >> 4, 31);

  return r_out | (g_out << 5) | (b_out << 10);
}

auto PPU::Brighten(u16 color) -> u16 {
  int evy = std::min<int>(16, mmio.evy);

  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  r += ((31 - r) * evy) >> 4;
  g += ((31 - g) * evy) >> 4;
  b += ((31 - b) * evy) >> 4;

  return r | (g << 5) | (b << 10);
}

auto PPU::Darken(u16 color) -> u16 {
  int evy = std::min<int>(16, mmio.evy);

  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  r -= (r * evy) >> 4;
  g -= (g * evy) >> 4;
  b -= (b * evy) >> 4;

  return r | (g << 5) | (b << 10);
}

} // namespace nba::core
