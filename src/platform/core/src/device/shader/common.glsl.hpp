/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

constexpr auto common_vert = R"(
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;

  void main() {
    v_uv = uv;
    gl_Position = vec4(position, 0.0, 1.0);
  }
)";

constexpr auto common_flip_vert = R"(
  #version 330 core

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec2 uv;

  out vec2 v_uv;

  void main() {
    v_uv = vec2(uv.x, 1.0 - uv.y);
    gl_Position = vec4(position, 0.0, 1.0);
  }
)";
