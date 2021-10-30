/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "device/shader/common.glsl.hpp"

constexpr auto output_vert = common_vert;

constexpr auto output_frag = R"(
  #version 330 core

  layout(location = 0) out vec4 frag_color;

  in vec2 v_uv;

  uniform sampler2D u_screen_map;

  void main() {
    frag_color = texture(u_screen_map, vec2(v_uv.x, 1.0 - v_uv.y));
  }
)";