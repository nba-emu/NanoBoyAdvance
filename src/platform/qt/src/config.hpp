/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <platform/config.hpp>
#include <Qt>
#include <vector>

#ifdef MACOS_BUILD_APP_BUNDLE
  #include <unistd.h>
  #include <pwd.h>
#endif

namespace fs = std::filesystem;

struct QtConfig final : nba::PlatformConfig {  
  QtConfig() {
    config_path = GetConfigPath();
  }

  struct Input {
    struct Map {
      int keyboard = 0;

      struct {
        int button = -1;
        int axis = -1;
      } controller{};

      static auto FromArray(std::array<int, 3> const& array) -> Map {
        return {array[0], {array[1], array[2]}};
      }

      auto Array() -> std::array<int, 3> {
        return {keyboard, controller.button, controller.axis};
      }
    };

    Map gba[nba::InputDevice::kKeyCount] {
      {Qt::Key_Up},
      {Qt::Key_Down},
      {Qt::Key_Left},
      {Qt::Key_Right},
      {Qt::Key_Return},
      {Qt::Key_Backspace},
      {Qt::Key_A},
      {Qt::Key_S},
      {Qt::Key_D},
      {Qt::Key_F}
    };

    Map fast_forward = {Qt::Key_Space};

    std::string controller_guid;
    bool hold_fast_forward = true;
  } input;

  struct Window {
    int scale = 2;
    int maximum_scale = 0;
    bool fullscreen = false;
    bool fullscreen_show_menu = false;
    bool lock_aspect_ratio = true;
    bool use_integer_scaling = false;
    bool show_fps = false;
  } window;

  std::vector<std::string> recent_files;

  void Load() {
    nba::PlatformConfig::Load(config_path);
  }

  void Save() {
    nba::PlatformConfig::Save(config_path);
  }

  void UpdateRecentFiles(std::u16string const& path) {
    const auto absolute_path = fs::absolute((fs::path)path).string();

    const auto match = std::find(recent_files.begin(), recent_files.end(), absolute_path);

    if(match != recent_files.end()) {
      recent_files.erase(match);
    }

    recent_files.insert(recent_files.begin(), absolute_path);

    if(recent_files.size() > 10) {
      recent_files.pop_back();
    }

    Save();
  }

protected:
  void LoadCustomData(toml::value const& data) override;

  void SaveCustomData(
    toml::basic_value<toml::preserve_comments>& data
  ) override;

private:
  auto GetConfigPath() const -> std::string {
    #ifdef MACOS_BUILD_APP_BUNDLE
      const auto pwd = getpwuid(getuid());

      if(pwd) {
        auto config_directory = fs::path{pwd->pw_dir} / "Library/Application Support/org.github.fleroviux.NanoBoyAdvance";

        if(!fs::exists(config_directory)) {
          fs::create_directory(config_directory);
        }

        return (config_directory / "config.toml").string();
      }
    #endif

    return "config.toml";
  }

  std::string config_path;
};
