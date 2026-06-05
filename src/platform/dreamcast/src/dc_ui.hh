// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "device/dc_video_device.hh"
#include "device/dc_input.hh"

#include <string>
#include <string_view>
#include <vector>

namespace nba {

struct DCUI {
  explicit DCUI(DCVideoDevice& video);

  void ClearScreen();
  void DrawText(int x, int y, std::string_view text);
  void DrawTextCentered(int y, std::string_view text);
  void DrawTextMultiline(int x, int y, std::string_view text);
  void DrawTitle(std::string_view title);
  void DrawMenu(
    std::string_view title,
    std::vector<std::string> const& items,
    int selection,
    int scroll_offset
  );
  void DrawStatusBar(std::string_view text);
  void DrawOverlay(std::string_view text);
  void Present();

  auto ShowMessage(
    std::string_view title,
    std::string_view message,
    DCInput& input,
    bool wait_for_start = true
  ) -> void;

  auto ShowFatalError(
    std::string_view message,
    DCInput& input
  ) -> void;

private:
  DCVideoDevice& video_;
};

} // namespace nba
