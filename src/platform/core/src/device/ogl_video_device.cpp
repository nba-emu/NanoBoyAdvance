/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <array>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <nba/log.hpp>
#include <platform/device/ogl_video_device.hpp>
#include <memory>

#include "device/shader/color_higan.glsl.hpp"
#include "device/shader/color_agb.glsl.hpp"
#include "device/shader/lcd_ghosting.glsl.hpp"
#include "device/shader/output.glsl.hpp"
#include "device/shader/xbrz.glsl.hpp"

using Video = nba::PlatformConfig::Video;

namespace nba {

static constexpr int gba_screen_width  = 240;
static constexpr int gba_screen_height = 160;

static constexpr std::array<GLfloat, 4 * 4> kQuadVertices = {
// position | UV coord
  -1, -1,     0, 0,
   1, -1,     1, 0,
  -1,  1,     0, 1,
   1,  1,     1, 1
};

OGLVideoDevice::OGLVideoDevice(std::shared_ptr<PlatformConfig> config) : config(config) {
}

OGLVideoDevice::~OGLVideoDevice() {
  ReleaseShaderPrograms();
  glDeleteVertexArrays(1, &quad_vao);
  glDeleteBuffers(1, &quad_vbo);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(textures.size(), textures.data());
}

void OGLVideoDevice::Initialize() {
  glewInit();

  // Create a fullscreen quad to render to the viewport.
  glGenVertexArrays(1, &quad_vao);
  glGenBuffers(1, &quad_vbo);
  glBindVertexArray(quad_vao);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // Create three textures and a framebuffer for postprocessing.
  glGenFramebuffers(1, &fbo);
  glGenTextures(textures.size(), textures.data());

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  ReloadConfig();
}

void OGLVideoDevice::ReloadConfig() {
  if(config->video.filter == Video::Filter::Linear) {
    texture_filter = GL_LINEAR;
  } else {
    texture_filter = GL_NEAREST;
  }

  UpdateTextures();
  CreateShaderPrograms();
}

void OGLVideoDevice::UpdateTextures() {
  for(const auto texture : textures) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gba_screen_width, gba_screen_height,
                 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
}

void OGLVideoDevice::CreateShaderPrograms() {
  auto const& video = config->video;

  ReleaseShaderPrograms();

  // xBRZ freescale upsampling filter (two passes)
  if(video.filter == Video::Filter::xBRZ) {
    auto [success0, program0] = CompileProgram(xbrz0_vert, xbrz0_frag);
    auto [success1, program1] = CompileProgram(xbrz1_vert, xbrz1_frag);
    
    if(success0 && success1) {
      programs.push_back(program0);
      programs.push_back(program1);
    } else {
      if(success0) glDeleteProgram(program0);
      if(success1) glDeleteProgram(program1);
    }
  }

  // Color correction pass
  switch(video.color) {
    case Video::Color::higan: {
      auto [success, program] = CompileProgram(color_higan_vert, color_higan_frag);
      if(success) {
        programs.push_back(program);
      }
      break;
    }
    case Video::Color::AGB: {
      auto [success, program] = CompileProgram(color_agb_vert, color_agb_frag);
      if(success) {
        programs.push_back(program);
      }
      break;
    }
  }

  // LCD ghosting (interframe blending) pass
  if(video.lcd_ghosting) {
    auto [success, program] = CompileProgram(lcd_ghosting_vert, lcd_ghosting_frag);
    if(success) {
      programs.push_back(program);
    }
  }

  // Output pass (final)
  auto [success, program] = CompileProgram(output_vert, output_frag);
  if(success) {
    programs.push_back(program);
  }

  UpdateShaderUniforms();
}

void OGLVideoDevice::UpdateShaderUniforms() {
  // Set constant shader uniforms.
  for(const auto program : programs) {
    glUseProgram(program);

    const auto input_map = glGetUniformLocation(program, "u_input_map");
    if(input_map != -1) {
      glUniform1i(input_map, 0);
    }

    const auto history_map = glGetUniformLocation(program, "u_history_map");
    if(history_map != -1) {
      glUniform1i(history_map, 1);
    }

    const auto source_map = glGetUniformLocation(program, "u_source_map");
    if(source_map != -1) {
      glUniform1i(source_map, 2);
    }

    const auto output_size = glGetUniformLocation(program, "u_output_size");
    if(output_size != -1) {
      glUniform2f(output_size, view_width, view_height);
    }
  }
}

void OGLVideoDevice::ReleaseShaderPrograms() {
  for(const auto program : programs) {
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
  
  if(!vert_success || !frag_success) {
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

  for(int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
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

  UpdateShaderUniforms();
}

void OGLVideoDevice::SetDefaultFBO(GLuint fbo) {
  default_fbo = fbo;
}

void OGLVideoDevice::Draw(u32* buffer) {
  int target = 0;

  // Update and bind LCD screen texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[3]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gba_screen_width, gba_screen_height,
               0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);

  // Bind LCD history map
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures[2]);

  // Bind LCD source map
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, textures[3]);

  auto program_count = programs.size();

  glViewport(0, 0, view_width, view_height);
  glBindVertexArray(quad_vao);

  if(program_count <= 2) {
    target = 2;
  }

  for(int i = 0; i < program_count; i++) {
    glUseProgram(programs[i]);

    if(i == program_count - 1) {
      glViewport(view_x, view_y, view_width, view_height);
      glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[target], 0);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Output of the current pass is the input for the next pass.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[target]);

    if(i == program_count - 3) {
      /* The next pass is the next-to-last pass, before we render to screen.
       * Render that pass into a separate texture, so that it can be 
       * used in the next frame to calculate LCD ghosting.
       */
      target = 2;
    } else {
      target ^= 1;
    }
  }
}

} // namespace nba
