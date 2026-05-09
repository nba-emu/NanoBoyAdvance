/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <nba/device/video_device.hpp>
#include <platform/config.hpp>
#include <glad/gl.h>
#include <array>
#include <utility>
#include <vector>

namespace nba {

struct OGLVideoDevice : VideoDevice {
  OGLVideoDevice(std::shared_ptr<PlatformConfig> config);
 ~OGLVideoDevice() override;

  void Initialize();
  void SetViewport(int x, int y, int width, int height);
  void SetDefaultFBO(GLuint fbo);
  void Draw(u32* buffer) override;
  void ReloadConfig();

private:
  void UpdateTextures();
  void CreateShaderPrograms();
  void UpdateShaderUniforms();
  void ReleaseShaderPrograms();

  auto CompileShader(
    GLenum type,
    char const* source
  ) -> std::pair<bool, GLuint>;

  auto CompileProgram(
    char const* vertex_src,
    char const* fragment_src
  ) -> std::pair<bool, GLuint>;

  static constexpr size_t input_index       = 0;
  static constexpr size_t output_index      = 1;
  static constexpr size_t history_index     = 2;
  static constexpr size_t xbrz_output_index = 3;

  struct ShaderPass {
    GLuint program = 0;
    struct {
      std::vector<GLuint> inputs = {input_index};
      GLuint output = output_index;
    } textures = {};
  };

  int view_x = 0;
  int view_y = 0;
  int view_width  = 1;
  int view_height = 1;
  GLuint default_fbo = 0;

  GLuint quad_vao;
  GLuint quad_vbo;
  GLuint fbo;
  std::array<GLuint, 4> textures        = {};
  std::vector<ShaderPass> shader_passes = {};
  GLint texture_filter = GL_NEAREST;

  std::shared_ptr<PlatformConfig> config;
};

} // namespace nba
