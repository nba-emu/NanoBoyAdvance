
#pragma once

#include "device/shader/common.glsl.hpp"

constexpr auto color_agb_vert = common_vert;

constexpr auto color_agb_frag = R"(\
  #version 330 core

  layout(location = 0) out vec4 frag_color;

  in vec2 v_uv;

  uniform sampler2D u_screen_map;

  void main() {
    vec4 color = texture(u_screen_map, v_uv);

    color.rgb = pow(color.rgb, vec3(4.0));

    frag_color = vec4(
      pow(
        vec3(
          1.000 * color.r + 0.196 * color.g,
          0.039 * color.r + 0.901 * color.g + 0.117 * color.b,
          0.196 * color.r + 0.039 * color.g + 0.862 * color.b
        ),
        vec3(1.0 / 2.2)
      ), 1.0);
  }
)";