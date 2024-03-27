/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "device/shader/common.glsl.hpp"

constexpr auto lcd_grid_vert = R"(
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;

  void main() {
    v_uv = uv * 1.0001;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)";

constexpr auto lcd_grid_frag = R"(
  #version 330 core

  #define BRIGHTEN_SCANLINES 16.0
  #define BRIGHTEN_LCD 24.0
  #define PI 3.141592653589793
  #define INPUT_SIZE vec2(240,160)


  layout(location = 0) out vec4 frag_color;

  in vec2 v_uv;

  uniform sampler2D u_input_map;

  void main() {
    vec2 angle = 2.0 * PI * ((v_uv.xy * INPUT_SIZE.xy) - 0.25);
    float yfactor = (BRIGHTEN_SCANLINES + sin(angle.y)) / (BRIGHTEN_SCANLINES + 1.0);
    float xfactor = (BRIGHTEN_LCD + sin(angle.x)) / (BRIGHTEN_LCD + 1.0);
    vec3 colour = texture(u_input_map, v_uv).rgb;
    colour.rgb = yfactor * xfactor * colour.rgb;

    frag_color = vec4(colour.rgb, 1.0);
  }
)";
