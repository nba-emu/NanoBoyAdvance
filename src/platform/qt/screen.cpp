/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QGLFunctions>

#include "screen.hpp"

void Screen::Draw(std::uint32_t* buffer) {
  emit SignalDraw(buffer);
}

void Screen::DrawSlot(std::uint32_t* buffer) {
  glBindTexture(GL_TEXTURE_2D, texture);

  /* Update texture pixels */
  glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGBA,
          240, /* TODO: do not hardcode the dimensions? */
          160,
          0,
          GL_BGRA,
          GL_UNSIGNED_BYTE,
          buffer
  );

  /* Redraw screen */
  update();
}

auto Screen::CompileShader() -> GLuint {
  QGLFunctions ctx(QGLContext::currentContext());

  auto vid = ctx.glCreateShader(GL_VERTEX_SHADER);
  auto fid = ctx.glCreateShader(GL_FRAGMENT_SHADER);

  const char* vert_src[] = {
    "varying vec2 uv;\n"
    "void main(void) {\n"
    "    gl_Position = gl_Vertex;\n"
    "    uv = gl_MultiTexCoord0;\n"
    "}"
  };

  const char* frag_src[] = {
    // Credits to Talarubi and byuu for the color correction algorithm.
    // https://byuu.net/video/color-emulation
    "uniform sampler2D tex;\n"
    "varying vec2 uv;\n"
    "void main(void) {\n"
    "    vec4 color = texture2D(tex, uv);\n"
    "    color.rgb = pow(color.rgb, vec3(4.0));\n"
    "    gl_FragColor.rgb = pow(vec3(color.r + 0.196 * color.g,\n"
    "                                0.039 * color.r + 0.901 * color.g + 0.117 * color.b,\n"
    "                                0.196 * color.r + 0.039 * color.g + 0.862 * color.b), vec3(1.0/2.2));"
    "    gl_FragColor.a = 1.0;\n"
    "}"
  };

  /* TODO: check for compilation errors. */
  ctx.glShaderSource(vid, 1, vert_src, nullptr);
  ctx.glShaderSource(fid, 1, frag_src, nullptr);
  ctx.glCompileShader(vid);
  ctx.glCompileShader(fid);

  auto pid = ctx.glCreateProgram();
  ctx.glAttachShader(pid, vid);
  ctx.glAttachShader(pid, fid);
  ctx.glLinkProgram(pid);

  return pid;
}

void Screen::initializeGL() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  program = CompileShader();
}

void Screen::paintGL() {
  QGLFunctions ctx(QGLContext::currentContext());

  glViewport(viewport_x, 0, viewport_width, viewport_height);
  glBindTexture(GL_TEXTURE_2D, texture);
  ctx.glUseProgram(program);

  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);
    glVertex2f(-1.0f, 1.0f);

    glTexCoord2f(1.0f, 0);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(0, 1.0f);
    glVertex2f(-1.0f, -1.0f);
  }
  glEnd();
}

void Screen::resizeGL(int width, int height) {
  viewport_width = height + height / 2;
  viewport_height = height;
  viewport_x = (width - viewport_width) / 2;
}
