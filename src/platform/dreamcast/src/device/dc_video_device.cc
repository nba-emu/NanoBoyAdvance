// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_video_device.hh"

#include <algorithm>
#include <cstring>

#if NBA_DC_HAS_KOS
#include <dc/video.h>
#include <dc/bfont.h>
#endif

namespace nba {

DCVideoDevice::DCVideoDevice() = default;

DCVideoDevice::~DCVideoDevice() = default;

bool DCVideoDevice::Initialize() {
#if NBA_DC_HAS_KOS
  vid_set_mode(DM_640x480, PM_RGB565);
  vram_base_ = (uint16*)vram_s;

  // Clear the screen to black
  std::memset(vram_base_, 0, kScreenWidth * kScreenHeight * sizeof(uint16));
#endif
  return true;
}

void DCVideoDevice::Draw(u32* buffer) {
#if NBA_DC_HAS_KOS
  if(!vram_base_ || !buffer) return;

  // Convert BGRA8888 GBA output to RGB565 and blit scaled 2x to VRAM.
  // The GBA framebuffer is 240x160; we scale to 480x320 centered on 640x480.
  for(int y = 0; y < kGBAHeight; y++) {
    for(int sy = 0; sy < kScale; sy++) {
      const int screen_y = kOffsetY + y * kScale + sy;
      uint16* dst = vram_base_ + screen_y * kScreenWidth + kOffsetX;

      for(int x = 0; x < kGBAWidth; x++) {
        // Core outputs BGRA8888 (B in low byte)
        const u32 pixel = buffer[y * kGBAWidth + x];
        const u8 b = (pixel >>  0) & 0xFF;
        const u8 g = (pixel >>  8) & 0xFF;
        const u8 r = (pixel >> 16) & 0xFF;

        const uint16 rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        for(int sx = 0; sx < kScale; sx++) {
          dst[x * kScale + sx] = rgb565;
        }
      }
    }
  }
#else
  (void)buffer;
#endif
}

void DCVideoDevice::ShowFatalError(const char* message) {
#if NBA_DC_HAS_KOS
  if(!vram_base_ || !message) return;

  std::memset(vram_base_, 0, kScreenWidth * kScreenHeight * sizeof(uint16));

  int y = kOffsetY;
  const char* line_start = message;

  while(*line_start) {
    const char* line_end = line_start;
    while(*line_end && *line_end != '\n') {
      line_end++;
    }

    char line[64];
    const auto line_len = std::min<size_t>(line_end - line_start, sizeof(line) - 1);
    std::memcpy(line, line_start, line_len);
    line[line_len] = '\0';

    bfont_draw_str_vram(kOffsetX, y, 0, line);
    y += 24;

    line_start = *line_end ? line_end + 1 : line_end;
  }

  vid_waitvbl();
#else
  (void)message;
#endif
}

} // namespace nba
