/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <nba/log.hpp>
#include <platform/device/ogl_video_device.hpp>
#include <memory>

#include "device/shader/color_agb.glsl.hpp"
#include "device/shader/color_ags.glsl.hpp"
#include "device/shader/lcd_ghosting.glsl.hpp"
#include "device/shader/output.glsl.hpp"
#include "device/shader/xbrz.glsl.hpp"

using Video = nba::PlatformConfig::Video;

namespace nba {

static const float kQuadVertices[] = {
// position | UV coord
  -1,  1,     0, 1,
   1,  1,     1, 1,
   1, -1,     1, 0,
   1, -1,     1, 0,
  -1, -1,     0, 0,
  -1,  1,     0, 1
};

OGLVideoDevice::OGLVideoDevice(std::shared_ptr<PlatformConfig> config) : config(config) {
}

OGLVideoDevice::~OGLVideoDevice() {
  ReleaseShaderPrograms();
  glDeleteVertexArrays(1, &quad_vao);
  glDeleteBuffers(1, &quad_vbo);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(4, texture);
}

void OGLVideoDevice::Initialize() {
  glewInit();

  // Create a fullscreen quad to render to the viewport.
  glGenVertexArrays(1, &quad_vao);
  glGenBuffers(1, &quad_vbo);
  glBindVertexArray(quad_vao);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // Create three textures and a framebuffer for postprocessing.
  glEnable(GL_TEXTURE_2D);
  glGenFramebuffers(1, &fbo);
  glGenTextures(4, texture);
  for (int i = 0; i < 4; i++) {
    glBindTexture(GL_TEXTURE_2D, texture[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  ReloadConfig();
}

void OGLVideoDevice::ReloadConfig() {
  texture_filter_invalid = true;

  if (config->video.filter == Video::Filter::Linear) {
    texture_filter = GL_LINEAR;
  } else {
    texture_filter = GL_NEAREST;
  }

  CreateShaderPrograms();
}

void OGLVideoDevice::CreateShaderPrograms() {
  auto const& video = config->video;

  ReleaseShaderPrograms();

  // xBRZ freescale upsampling filter (two passes)
  if (video.filter == Video::Filter::xBRZ) {
    auto [success0, program0] = CompileProgram(xbrz0_vert, xbrz0_frag);
    auto [success1, program1] = CompileProgram(xbrz1_vert, xbrz1_frag);
    
    if (success0 && success1) {
      programs.push_back(program0);
      programs.push_back(program1);
    } else {
      if (success0) glDeleteProgram(program0);
      if (success1) glDeleteProgram(program1);
    }
  }

  // Color correction pass
  switch (video.color) {
    case Video::Color::AGB: {
      auto [success, program] = CompileProgram(color_agb_vert, color_agb_frag);
      if (success) {
        programs.push_back(program);
      }
      break;
    }
    case Video::Color::AGS: {
      auto [success, program] = CompileProgram(color_ags_vert, color_ags_frag);
      if (success) {
        programs.push_back(program);
      }
      break;
    }
  }

  // LCD ghosting (interframe blending) pass
  if (video.lcd_ghosting) {
    auto [success, program] = CompileProgram(lcd_ghosting_vert, lcd_ghosting_frag);
    if (success) {
      programs.push_back(program);
    }
  }

  // Output pass (final)
  auto [success, program] = CompileProgram(output_vert, output_frag);
  if (success) {
    programs.push_back(program);
  }
}

void OGLVideoDevice::ReleaseShaderPrograms() {
  for (auto program : programs) {   
    glDeleteProgram(program);
  }
  programs.clear();
}

auto OGLVideoDevice::CompileShader(
  GLenum type,
  char const* source
) -> std::pair<bool, GLuint> {
  char const* source_array[] = { source };
  
  auto shader = glCreateShader(type);

  glShaderSource(shader, 1, source_array, nullptr);
  glCompileShader(shader);
  
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(compiled == GL_FALSE) {
    GLint max_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

    auto error_log = std::make_unique<GLchar[]>(max_length);
    glGetShaderInfoLog(shader, max_length, &max_length, error_log.get());
    Log<Error>("OGLVideoDevice: failed to compile shader:\n{0}", error_log.get());
    return std::make_pair(false, shader);
  }

  return std::make_pair(true, shader);
}

auto OGLVideoDevice::CompileProgram(
  char const* vertex_src,
  char const* fragment_src
) -> std::pair<bool, GLuint> {
  auto [vert_success, vert_id] = CompileShader(GL_VERTEX_SHADER, vertex_src);
  auto [frag_success, frag_id] = CompileShader(GL_FRAGMENT_SHADER, fragment_src);
  
  if (!vert_success || !frag_success) {
    return std::make_pair<bool, GLuint>(false, 0);
  } else {
    auto prog_id = glCreateProgram();

    glAttachShader(prog_id, vert_id);
    glAttachShader(prog_id, frag_id);
    glLinkProgram(prog_id);
    glDeleteShader(vert_id);
    glDeleteShader(frag_id);

    return std::make_pair(true, prog_id);
  }
}

void OGLVideoDevice::SetViewport(int x, int y, int width, int height) {
  view_x = x;
  view_y = y;
  view_width  = width;
  view_height = height;

  for (int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, texture[i]);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      view_width,
      view_height,
      0,
      GL_BGRA,
      GL_UNSIGNED_BYTE,
      nullptr
    );
  }
}

void OGLVideoDevice::SetDefaultFBO(GLuint fbo) {
  default_fbo = fbo;
}

void OGLVideoDevice::Draw(u32* buffer) {
  int target = 0;

  glViewport(0, 0, view_width, view_height);
  glBindVertexArray(quad_vao);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[2]); // history map

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture[3]); // LCD source map

  auto program_count = programs.size();

  for (int i = 0; i < program_count; i++) {
    auto program_id = programs[i];

    if (i == program_count - 1) {
      // Copy output of current frame to the history buffer.
      // TODO: this is a bit inefficient.
      glReadBuffer(GL_COLOR_ATTACHMENT0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture[2]);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, view_width, view_height);

      glViewport(view_x, view_y, view_width, view_height);
      glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[target], 0);
    }

    glUseProgram(program_id);

    glActiveTexture(GL_TEXTURE0);

    if (i == 0) {
      glBindTexture(GL_TEXTURE_2D, texture[3]);
      glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        240,
        160,
        0,
        GL_BGRA,
        GL_UNSIGNED_BYTE,
        buffer
      );
      if (texture_filter_invalid) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter);
        texture_filter_invalid = false;
      }
    } else {
      glBindTexture(GL_TEXTURE_2D, texture[target ^ 1]);
    }

    auto screen_map = glGetUniformLocation(program_id, "u_screen_map");
    if (screen_map != -1) {
      glUniform1i(screen_map, 0);
    }

    auto history_map = glGetUniformLocation(program_id, "u_history_map");
    if (history_map != -1) {
      glUniform1i(history_map, 1);
    }

    auto source_map = glGetUniformLocation(program_id, "u_source_map");
    if (source_map != -1) {
      glUniform1i(source_map, 2);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
    target ^= 1;
  }
}

} // namespace nba
