/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/device/video_device.hpp>
#include <GL/glew.h>
#include <platform/config.hpp>
#include <string>
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
  void CreateShaderPrograms();
  void ReleaseShaderPrograms();

  auto CompileShader(
    GLenum type,
    char const* source
  ) -> std::pair<bool, GLuint>;
  
  auto CompileProgram(
    char const* vertex_src,
    char const* fragment_src
  ) -> std::pair<bool, GLuint>;

  int view_x = 0;
  int view_y = 0;
  int view_width  = 1;
  int view_height = 1;
  GLuint default_fbo = 0;

  GLuint quad_vao;
  GLuint quad_vbo;
  GLuint fbo;
  GLuint texture[4];
  std::vector<GLuint> programs;
  GLenum texture_filter = GL_NEAREST;
  bool texture_filter_invalid = false;

  std::shared_ptr<PlatformConfig> config;
};

} // namespace nba
