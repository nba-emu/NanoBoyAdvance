/*
 * Copyright (C) 2020 fleroviux
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
#include <SDL.h>
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
        int button = SDL_CONTROLLER_BUTTON_INVALID;
        int axis = SDL_CONTROLLER_AXIS_INVALID;
      } controller;

      static auto FromArray(std::array<int, 3> const& array) -> Map {
        return {array[0], {array[1], array[2]}};
      }

      auto Array() -> std::array<int, 3> {
        return {keyboard, controller.button, controller.axis};
      }
    };

    Map gba[nba::InputDevice::kKeyCount] {
      {Qt::Key_Up, {SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_AXIS_LEFTY | 0x80}},
      {Qt::Key_Down, {SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_AXIS_LEFTY}},
      {Qt::Key_Left, {SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_AXIS_LEFTX | 0x80}},
      {Qt::Key_Right, {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_AXIS_LEFTX}},
      {Qt::Key_Return, {SDL_CONTROLLER_BUTTON_START}},
      {Qt::Key_Backspace, {SDL_CONTROLLER_BUTTON_BACK}},
      {Qt::Key_A, {SDL_CONTROLLER_BUTTON_A}},
      {Qt::Key_S, {SDL_CONTROLLER_BUTTON_B}},
      {Qt::Key_D, {SDL_CONTROLLER_BUTTON_LEFTSHOULDER}},
      {Qt::Key_F, {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER}}
    };

    Map fast_forward = {Qt::Key_Space, {SDL_CONTROLLER_BUTTON_X}};

    std::string controller_guid;
    bool hold_fast_forward = true;
  } input;

  struct Window {
    int scale = 2;
    int maximum_scale = 0;
    bool fullscreen = false;
    bool fullscreen_show_menu = false;
    bool lock_aspect_ratio = true;
    bool snap_to_integer_scale = false;
    bool show_fps = false;
  } window;

  std::vector<std::string> recent_files;

  void Load() {
    nba::PlatformConfig::Load(config_path);
  }

  void Save() {
    nba::PlatformConfig::Save(config_path);
  }

  void UpdateRecentFiles(std::string const& path) {
    auto absolute_path = fs::absolute((fs::path)path).string();

    auto match = std::find(recent_files.begin(), recent_files.end(), absolute_path);

    if (match != recent_files.end()) {
      recent_files.erase(match);
    }

    recent_files.insert(recent_files.begin(), absolute_path);

    if (recent_files.size() > 10) {
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

      if (pwd) {
        auto config_directory = fs::path{pwd->pw_dir} / "Library/Application Support/org.github.fleroviux.NanoBoyAdvance";

        if (!fs::exists(config_directory)) {
          fs::create_directory(config_directory);
        }

        return (config_directory / "config.toml").string();
      }
    #endif

    return "config.toml";
  }

  std::string config_path;
};
