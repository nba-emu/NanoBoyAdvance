// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_ui.hh"

#include <algorithm>
#include <cstring>

namespace nba {

DCUI::DCUI(DCVideoDevice& video) : video_(video) {}

void DCUI::ClearScreen() {
  video_.ClearScreen();
}

void DCUI::DrawText(int x, int y, std::string_view text) {
  video_.DrawText(x, y, text);
}

void DCUI::DrawTextCentered(int y, std::string_view text) {
  video_.DrawTextCentered(y, text);
}

void DCUI::DrawTextMultiline(int x, int y, std::string_view text) {
  video_.DrawTextMultiline(x, y, text);
}

void DCUI::DrawTitle(std::string_view title) {
  DrawTextCentered(24, title);
}

void DCUI::DrawMenu(
  std::string_view title,
  std::vector<std::string> const& items,
  int selection,
  int scroll_offset
) {
  ClearScreen();
  DrawTitle(title);

  static constexpr int kVisibleRows = 10;
  static constexpr int kRowHeight = 24;
  static constexpr int kListTop = 72;

  const int item_count = static_cast<int>(items.size());
  const int visible = std::min(item_count - scroll_offset, kVisibleRows);

  for(int row = 0; row < visible; row++) {
    const int index = scroll_offset + row;
    const int y = kListTop + row * kRowHeight;
    const bool selected = index == selection;
    const char* prefix = selected ? "> " : "  ";
    std::string line = std::string{prefix} + items[index];
    DrawText(48, y, line);
  }

  if(item_count == 0) {
    DrawTextCentered(kListTop, "No items found");
  }

  DrawStatusBar("A=Select  B=Back  Y=Settings  Start=Loader");
  Present();
}

void DCUI::DrawStatusBar(std::string_view text) {
  video_.DrawStatusBar(text);
}

void DCUI::DrawOverlay(std::string_view text) {
  video_.DrawOverlay(text);
}

void DCUI::Present() {
  video_.Present();
}

void DCUI::ShowMessage(
  std::string_view title,
  std::string_view message,
  DCInput& input,
  bool wait_for_start
) {
  ClearScreen();
  DrawTitle(title);
  DrawTextMultiline(48, 96, message);

  if(wait_for_start) {
    DrawStatusBar("Press Start to continue");
  }

  Present();

  if(!wait_for_start) {
    return;
  }

  input.WaitForButton(DCInput::Button::Start);
}

void DCUI::ShowFatalError(std::string_view message, DCInput& input) {
  ShowMessage("Error", message, input, true);
}

} // namespace nba
