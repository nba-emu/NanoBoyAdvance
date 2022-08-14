/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "config.hpp"

void QtConfig::LoadCustomData(toml::value const& data) {
  if (data.contains("input")) {
    auto input_result = toml::expect<toml::value>(data.at("input"));

    if (input_result.is_ok()) {
      using Map = Input::Map;

      auto input_ = input_result.unwrap();

      input.controller_guid = toml::find_or<std::string>(input_, "controller_guid", "");
      input.hold_fast_forward = toml::find_or<bool>(input_, "hold_fast_forward", true);

      const auto get_map = [&](toml::value const& value, std::string key) {
        return Map::FromArray(toml::find_or<std::array<int, 3>>(value, key, {
          0, SDL_CONTROLLER_BUTTON_INVALID, SDL_CONTROLLER_AXIS_INVALID}));
      };

      input.fast_forward = get_map(input_, "fast_forward");
    
      if (input_.contains("gba")) {
        auto gba_result = toml::expect<toml::value>(input_.at("gba"));

        if (gba_result.is_ok()) {
          auto gba = gba_result.unwrap();

          input.gba[0] = get_map(gba, "up");
          input.gba[1] = get_map(gba, "down");
          input.gba[2] = get_map(gba, "left");
          input.gba[3] = get_map(gba, "right");
          input.gba[4] = get_map(gba, "start");
          input.gba[5] = get_map(gba, "select");
          input.gba[6] = get_map(gba, "a");
          input.gba[7] = get_map(gba, "b");
          input.gba[8] = get_map(gba, "l");
          input.gba[9] = get_map(gba, "r");
        }
      }
    }
  }

  if (data.contains("window")) {
    auto window_result = toml::expect<toml::value>(data.at("window"));

    if (window_result.is_ok()) {
      auto window_ = window_result.unwrap();

      window.maximum_scale = toml::find_or<int>(window_, "maximum_scale", 0);
      window.show_fps = toml::find_or<bool>(window_, "show_fps", false);
      window.lock_aspect_ratio = toml::find_or<bool>(window_, "lock_aspect_ratio", true);
      window.snap_to_integer_scale = toml::find_or<bool>(window_, "snap_to_integer_scale", false);
    }
  }

  recent_files = toml::find_or<std::vector<std::string>>(data, "recent_files", {});
}

void QtConfig::SaveCustomData(
  toml::basic_value<toml::preserve_comments>& data
) {
  data["input"]["controller_guid"] = input.controller_guid;
  data["input"]["fast_forward"] = input.fast_forward.Array();
  data["input"]["hold_fast_forward"] = input.hold_fast_forward;

  data["input"]["gba"]["up"] = input.gba[0].Array();
  data["input"]["gba"]["down"] = input.gba[1].Array();
  data["input"]["gba"]["left"] = input.gba[2].Array();
  data["input"]["gba"]["right"] = input.gba[3].Array();
  data["input"]["gba"]["start"] = input.gba[4].Array();
  data["input"]["gba"]["select"] = input.gba[5].Array();
  data["input"]["gba"]["a"] = input.gba[6].Array();
  data["input"]["gba"]["b"] = input.gba[7].Array();
  data["input"]["gba"]["l"] = input.gba[8].Array();
  data["input"]["gba"]["r"] = input.gba[9].Array();

  data["window"]["maximum_scale"] = window.maximum_scale;
  data["window"]["show_fps"] = window.show_fps;
  data["window"]["lock_aspect_ratio"] = window.lock_aspect_ratio;
  data["window"]["snap_to_integer_scale"] = window.snap_to_integer_scale;

  data["recent_files"] = recent_files;
}
