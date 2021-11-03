/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "device/shader/common.glsl.hpp"

constexpr auto lcd_ghosting_vert = common_vert;

constexpr auto lcd_ghosting_frag = R"(
  #version 330 core

  layout(location = 0) out vec4 frag_color;

  in vec2 v_uv;

  uniform sampler2D u_screen_map;
  uniform sampler2D u_history_map;

  void main() {
    vec4 color_a = texture(u_screen_map, v_uv);
    vec4 color_b = texture(u_history_map, v_uv);
    frag_color = mix(color_a, color_b, 0.5);
  }
)";