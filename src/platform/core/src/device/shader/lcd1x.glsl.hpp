/*
 * lcd1x - Simple LCD 'scanline' shader, based on lcd3x
 *
 * Original code by Gigaherz, released into the public domain
 *
 * Ported to NanoBoyAdvance by Destiny
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * lcd1x differs from lcd3x in the following manner:
 *
 * > Omits LCD-style colour separation
 *
 * > Has 'correctly' aligned scanlines
 */

#pragma once

constexpr auto lcd1x_vert = R"(
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;

  void main() {
    v_uv = vec2(uv.x, 1.0 - uv.y) * 1.0001;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)";

constexpr auto lcd1x_frag = R"(
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
