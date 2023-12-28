/*
   Author: rsn8887 (based on TheMaister)
   License: Public domain

   This is an integer prescale filter that should be combined
   with a bilinear hardware filtering (GL_BILINEAR filter or some such) to achieve
   a smooth scaling result with minimum blur. This is good for pixelgraphics
   that are scaled by non-integer factors.

   The prescale factor and texel coordinates are precalculated
   in the vertex shader for speed.
*/

#pragma once

constexpr auto sharp_bilinear_vert = R"(
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;
  out vec2 precalc_texel;
  out vec2 precalc_scale;

  uniform vec2 u_output_size;

  void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    v_uv = vec2(uv.x, 1.0 - uv.y);

    const vec2 input_size = vec2(240, 160);
    precalc_scale = max(floor(u_output_size / input_size), vec2(1.0, 1.0));
    precalc_texel = v_uv * input_size;
  }
)";

constexpr auto sharp_bilinear_frag = R"(
  #version 330 core

  in vec2 v_uv;
  in vec2 precalc_texel;
  in vec2 precalc_scale;

  layout(location = 0) out vec4 frag_color;

  uniform sampler2D u_input_map;

  void main() {
    vec2 texel = precalc_texel;
    vec2 scale = precalc_scale;
    vec2 texel_floored = floor(texel);
    vec2 s = fract(texel);
    vec2 region_range = 0.5 - 0.5 / scale;

    // Figure out where in the texel to sample to get correct pre-scaled bilinear.
    // Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.

    vec2 center_dist = s - 0.5;
    vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

    vec2 mod_texel = texel_floored + f;

    const vec2 input_size = vec2(240, 160);
    frag_color = vec4(texture(u_input_map, mod_texel / input_size).rgb, 1.0);
  }
)";
