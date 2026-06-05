// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_video_device.hh"

#include <algorithm>
#include <array>
#include <cstring>

#include "../font_8x16.hh"

#if NBA_DC_HAS_KOS
#include <dc/video.h>
#endif

namespace nba {

DCVideoDevice::DCVideoDevice() = default;

DCVideoDevice::~DCVideoDevice() = default;

bool DCVideoDevice::Initialize() {
#if NBA_DC_HAS_KOS
  vid_set_mode(DM_640x480, PM_RGB565);
  vram_base_ = (uint16*)vram_s;
  ClearScreen();
#endif
  return true;
}

void DCVideoDevice::ClearScreen() {
#if NBA_DC_HAS_KOS
  if(!vram_base_) return;
  std::memset(vram_base_, 0, kScreenWidth * kScreenHeight * sizeof(uint16));
#endif
}

void DCVideoDevice::DrawText(int x, int y, std::string_view text) {
#if NBA_DC_HAS_KOS
  if(!vram_base_) return;

  const uint16 fg = 0xFFFF;
  const uint16 bg = 0x0000;

  int cursor_x = x;
  int cursor_y = y;
  for(char c : text) {
    if(c < 32 || c > 126) {
      cursor_x += kFontWidth;
      continue;
    }
    const auto& glyph = kFont8x16[c - 32];
    for(int row = 0; row < static_cast<int>(kFontHeight); row++) {
      const std::uint8_t bits = glyph[row];
      uint16* dst = vram_base_ + (cursor_y + row) * kScreenWidth + cursor_x;
      for(int col = 0; col < static_cast<int>(kFontWidth); col++) {
        const bool on = (bits >> (7 - col)) & 1;
        dst[col] = on ? fg : bg;
      }
    }
    cursor_x += kFontWidth;
  }
#else
  (void)x;
  (void)y;
  (void)text;
#endif
}

void DCVideoDevice::DrawTextCentered(int y, std::string_view text) {
  const int x = std::max(0, kOffsetX + ((kGBAWidth * kScale) / 2) - (static_cast<int>(text.size()) * kFontWidth) / 2);
  DrawText(x, y, text);
}

void DCVideoDevice::DrawTextMultiline(int x, int y, std::string_view text) {
  while(!text.empty()) {
    const auto newline = text.find('\n');
    const auto line = text.substr(0, newline);
    DrawText(x, y, line);
    y += kLineHeight;

    if(newline == std::string_view::npos) {
      break;
    }

    text.remove_prefix(newline + 1);
  }
}

void DCVideoDevice::DrawStatusBar(std::string_view text) {
#if NBA_DC_HAS_KOS
  if(!vram_base_) return;

  for(int x = 0; x < kScreenWidth; x++) {
    vram_base_[kStatusBarY * kScreenWidth + x] = 0x1084;
  }
#endif

  DrawText(16, kStatusBarY + 4, text);
}

void DCVideoDevice::DrawOverlay(std::string_view text) {
  DrawStatusBar(text);
  Present();
}

void DCVideoDevice::Present() {
#if NBA_DC_HAS_KOS
  vid_waitvbl();
#endif
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
  ClearScreen();
  DrawTextMultiline(kOffsetX, kOffsetY, message ? message : "Unknown error");
  Present();
}

} // namespace nba
