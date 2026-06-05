// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/device/video_device.hh>

#include <string_view>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
#else
#define NBA_DC_HAS_KOS 0
#endif

namespace nba {

// Dreamcast video device using PVR direct-frame rendering.
// Renders the GBA 240x160 framebuffer centered on a 640x480 display
// using software blitting to the PVR framebuffer.
struct DCVideoDevice : VideoDevice {
  DCVideoDevice();
  ~DCVideoDevice() override;

  bool Initialize();
  void Draw(u32* buffer) override;
  void ShowFatalError(const char* message);

  void ClearScreen();
  void DrawText(int x, int y, std::string_view text);
  void DrawTextCentered(int y, std::string_view text);
  void DrawTextMultiline(int x, int y, std::string_view text);
  void DrawStatusBar(std::string_view text);
  void DrawOverlay(std::string_view text);
  void Present();

private:
  static constexpr int kGBAWidth  = 240;
  static constexpr int kGBAHeight = 160;
  static constexpr int kScreenWidth  = 640;
  static constexpr int kScreenHeight = 480;
  static constexpr int kScale = 2;
  static constexpr int kOffsetX = (kScreenWidth  - kGBAWidth  * kScale) / 2;
  static constexpr int kOffsetY = (kScreenHeight - kGBAHeight * kScale) / 2;
  static constexpr int kFontWidth = 12;
  static constexpr int kLineHeight = 24;
  static constexpr int kStatusBarY = 448;

#if NBA_DC_HAS_KOS
  uint16* vram_base_ = nullptr;
#endif
};

} // namespace nba
