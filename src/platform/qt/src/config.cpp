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
      auto input_ = input_result.unwrap();

      input.fast_forward = toml::find_or<int>(input_, "fast_forward", Qt::Key_Space);
      input.hold_fast_forward = toml::find_or<bool>(input_, "hold_fast_forward", true);
    
      if (input_.contains("gba")) {
        auto gba_result = toml::expect<toml::value>(input_.at("gba"));

        if (gba_result.is_ok()) {
          auto gba = gba_result.unwrap();
          input.gba[0] = toml::find_or<int>(gba, "up", Qt::Key_Up);
          input.gba[1] = toml::find_or<int>(gba, "down", Qt::Key_Down);
          input.gba[2] = toml::find_or<int>(gba, "left", Qt::Key_Left);
          input.gba[3] = toml::find_or<int>(gba, "right", Qt::Key_Right);
          input.gba[4] = toml::find_or<int>(gba, "start", Qt::Key_Return);
          input.gba[5] = toml::find_or<int>(gba, "select", Qt::Key_Backspace);
          input.gba[6] = toml::find_or<int>(gba, "a", Qt::Key_A);
          input.gba[7] = toml::find_or<int>(gba, "b", Qt::Key_S);
          input.gba[8] = toml::find_or<int>(gba, "l", Qt::Key_D);
          input.gba[9] = toml::find_or<int>(gba, "r", Qt::Key_F);
        }
      }
    }
  }
}

void QtConfig::SaveCustomData(
  toml::basic_value<toml::preserve_comments>& data
) {
  data["input"]["fast_forward"] = input.fast_forward;
  data["input"]["hold_fast_forward"] = input.hold_fast_forward;

  data["input"]["gba"]["up"] = input.gba[0];
  data["input"]["gba"]["down"] = input.gba[1];
  data["input"]["gba"]["left"] = input.gba[2];
  data["input"]["gba"]["right"] = input.gba[3];
  data["input"]["gba"]["start"] = input.gba[4];
  data["input"]["gba"]["select"] = input.gba[5];
  data["input"]["gba"]["a"] = input.gba[6];
  data["input"]["gba"]["b"] = input.gba[7];
  data["input"]["gba"]["l"] = input.gba[8];
  data["input"]["gba"]["r"] = input.gba[9];
}