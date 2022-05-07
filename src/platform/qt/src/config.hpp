/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <filesystem>
#include <platform/config.hpp>
#include <Qt>

struct QtConfig final : nba::PlatformConfig {  
  struct Input {
    int gba[nba::InputDevice::kKeyCount] {
      Qt::Key_Up,
      Qt::Key_Down,
      Qt::Key_Left,
      Qt::Key_Right,
      Qt::Key_Return,
      Qt::Key_Backspace,
      Qt::Key_A,
      Qt::Key_S,
      Qt::Key_D,
      Qt::Key_F
    };

    int fast_forward = Qt::Key_Space;
    bool hold_fast_forward = true;
  } input;

  struct Window {
    bool show_fps = false;
  } window;

  void Load() {
    nba::PlatformConfig::Load(kConfigPath);
  }

  void Save() {
    namespace fs = std::filesystem;

    auto directory = fs::path{kConfigPath}.remove_filename();

    if (!directory.empty() && !fs::exists(directory)) {
      fs::create_directory(directory);
    }

    nba::PlatformConfig::Save(kConfigPath);
  }

protected:
  void LoadCustomData(toml::value const& data) override;

  void SaveCustomData(
    toml::basic_value<toml::preserve_comments>& data
  ) override;

private:

#ifdef MACOS_BUILD_APP_BUNDLE
  static constexpr auto kConfigPath = "~/Library/Application Support/org.github.fleroviux.NanoBoyAdvance/config.toml";
#else
  static constexpr auto kConfigPath = "config.toml";
#endif
};
