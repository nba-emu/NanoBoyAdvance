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

#ifdef MACOS_BUILD_APP_BUNDLE
  #include <pwd.h>
#endif

struct QtConfig final : nba::PlatformConfig {  
  QtConfig() {
    config_path = GetConfigPath();
  }

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
    nba::PlatformConfig::Load(config_path);
  }

  void Save() {
    nba::PlatformConfig::Save(config_path);
  }

protected:
  void LoadCustomData(toml::value const& data) override;

  void SaveCustomData(
    toml::basic_value<toml::preserve_comments>& data
  ) override;

private:
  auto GetConfigPath() const -> std::string {
    namespace fs = std::filesystem;

    #ifdef MACOS_BUILD_APP_BUNDLE
      const auto pwd = getpwuid(getuid());

      if (pwd) {
        auto config_directory = fs::path{pwd->pw_dir} + "Library/Application Support/org.github.fleroviux.NanoBoyAdvance/";

        if (!fs::exists(config_directory)) {
          fs::create_directory(config_directory);
        }

        return (config_directory + "config.toml").string();
      }
    #endif

    return "config.toml";
  }

  std::string config_path;
};
