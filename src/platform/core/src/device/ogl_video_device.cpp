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

namespace nba {

static const float kQuadVertices[] = {
// position | UV coord
  -1,  1,     0, 1,
   1,  1,     1, 1,
   1, -1,     1, 0,
  -1, -1,     0, 0
};

OGLVideoDevice::~OGLVideoDevice() {
  // TODO: delete and programs that were created.
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

  // Compile shader test
  {
    char const* vert_src[] = {
      R"(\
#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 v_uv;

void main() {
  v_uv = uv;
  gl_Position = vec4(position, 0.0, 1.0);
})"
    };

    char const* frag_src[] = {
      R"(\
#version 330 core

/*
   Shader Modified: Pokefan531
   Color Mangler
   Author: hunterk
   License: Public domain
*/

// Shader that replicates the LCD dynamics from a GameBoy Advance

#define darken_screen 1.0
#define target_gamma 2.2
#define display_gamma 2.2
#define sat 1.0
#define lum 0.94
#define contrast 1.0
#define blr 0.0
#define blg 0.0
#define blb 0.0
#define r 0.82
#define g 0.665
#define b 0.73
#define rg 0.125
#define rb 0.195
#define gr 0.24
#define gb 0.075
#define br -0.06
#define bg 0.21
#define overscan_percent_x 0.0
#define overscan_percent_y 0.0

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_screen_map;

void main() {
  vec4 screen = pow(texture2D(u_screen_map, v_uv), vec4(target_gamma + darken_screen)).rgba;
  vec4 avglum = vec4(0.5);
  screen = mix(screen, avglum, (1.0 - contrast));
   
  mat4 color = mat4(
    r,   rg,  rb,  0.0,  //red channel
    gr,  g,   gb,  0.0,  //green channel
    br,  bg,  b,   0.0,  //blue channel
    blr, blg, blb, 0.0   //alpha channel; these numbers do nothing for our purposes.
  );
        
  mat4 adjust = mat4(
    (1.0 - sat) * 0.3086 + sat, (1.0 - sat) * 0.3086, (1.0 - sat) * 0.3086, 1.0,
    (1.0 - sat) * 0.6094, (1.0 - sat) * 0.6094 + sat, (1.0 - sat) * 0.6094, 1.0,
    (1.0 - sat) * 0.0820, (1.0 - sat) * 0.0820, (1.0 - sat) * 0.0820 + sat, 1.0,
    0.0, 0.0, 0.0, 1.0
  );

  color *= adjust;
  screen = clamp(screen * lum, 0.0, 1.0);
  screen = color * screen;

  frag_color = pow(screen, vec4(1.0 / display_gamma));
})",
      R"(
#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_screen_map;
uniform sampler2D u_history_map;

void main() {
  vec4 color_a = texture2D(u_screen_map, v_uv);
  vec4 color_b = texture2D(u_history_map, v_uv);
  frag_color = mix(color_a, color_b, 0.5);
}
)",
      R"(
#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_screen_map;

void main() {
  frag_color = texture2D(u_screen_map, vec2(v_uv.x, 1.0 - v_uv.y));
})"
    };

    program_a = CompileProgram(vert_src[0], frag_src[0]).second;
    program_b = CompileProgram(vert_src[0], frag_src[1]).second;
    program_out = CompileProgram(vert_src[0], frag_src[2]).second;
  }  

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
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
  GLuint programs[] = { program_a, program_b, program_out };
  auto program_count = sizeof(programs) / sizeof(GLuint);

  int target = 0;

  glViewport(0, 0, view_width, view_height);
  glBindVertexArray(quad_vao);

  // Bind history buffer in the second texture slot.
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[2]);

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

    glDrawArrays(GL_QUADS, 0, 4);
    target ^= 1;
  }
}

} // namespace nba
