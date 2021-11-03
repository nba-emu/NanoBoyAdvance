/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

constexpr auto common_vert = R"(\
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;

  void main() {
    v_uv = uv;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)";