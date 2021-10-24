/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <GL/glew.h>
// #include <QGLFunctions>

#include "widget/screen.hpp"

Screen::Screen(QWidget* parent)
    : QOpenGLWidget(parent) {
  QSurfaceFormat format;
  format.setSwapInterval(0);
  setFormat(format);
  connect(this, &Screen::RequestDraw, this, &Screen::OnRequestDraw);
}

Screen::~Screen() {
  // glDeleteTextures(1, &texture);
}

void Screen::Draw(u32* buffer) {
  should_clear = false;
  emit RequestDraw(buffer);
}

void Screen::Clear() {
  should_clear = true;
  update();
}

void Screen::OnRequestDraw(u32* buffer) {
  // glBindTexture(GL_TEXTURE_2D, texture);
  // glTexImage2D(
  //   GL_TEXTURE_2D,
  //   0,
  //   GL_RGBA,
  //   240,
  //   160,
  //   0,
  //   GL_BGRA,
  //   GL_UNSIGNED_BYTE,
  //   buffer
  // );
  this->buffer = buffer;
  update();
}

auto Screen::CreateShader() -> GLuint {
  // auto vid = glCreateShader(GL_VERTEX_SHADER);
  // auto fid = glCreateShader(GL_FRAGMENT_SHADER);

  // const char* vert_src[] = {
  //   "varying vec2 uv;\n"
  //   "void main(void) {\n"
  //   "    gl_Position = gl_Vertex;\n"
  //   "    uv = gl_MultiTexCoord0;\n"
  //   "}"
  // };

  // const char* frag_src[] = {
  //   "uniform sampler2D tex;\n"
  //   "varying vec2 uv;\n"
  //   "void main(void) {\n"
  //   "    vec4 color = texture2D(tex, uv);\n"
  //   "    color.rgb = pow(color.rgb, vec3(4.0));\n"
  //   "    gl_FragColor.rgb = pow(vec3(color.r + 0.196 * color.g,\n"
  //   "                                0.039 * color.r + 0.901 * color.g + 0.117 * color.b,\n"
  //   "                                0.196 * color.r + 0.039 * color.g + 0.862 * color.b), vec3(1.0/2.2));"
  //   "    gl_FragColor.a = 1.0;\n"
  //   "}"
  // };

  // // TODO: check for and log shader compilation errors.
  // glShaderSource(vid, 1, vert_src, nullptr);
  // glShaderSource(fid, 1, frag_src, nullptr);
  // glCompileShader(vid);
  // glCompileShader(fid);

  // auto pid = glCreateProgram();
  // glAttachShader(pid, vid);
  // glAttachShader(pid, fid);
  // glLinkProgram(pid);

  // return pid;

  return 0;
}

void Screen::initializeGL() {
  makeCurrent();
  ogl_video_device.Initialize();

  // glewInit();

  // glMatrixMode(GL_PROJECTION);
  // glLoadIdentity();

  // glMatrixMode(GL_MODELVIEW);
  // glLoadIdentity();

  // glEnable(GL_TEXTURE_2D);
  // glGenTextures(1, &texture);
  // glBindTexture(GL_TEXTURE_2D, texture);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // glClearColor(0, 0, 0, 1);
  // glClear(GL_COLOR_BUFFER_BIT);

  // program = CreateShader();
}

void Screen::paintGL() {
  // glViewport(viewport_x, 0, viewport_width, viewport_height);

  // if (!should_clear) {
  //   glBindTexture(GL_TEXTURE_2D, texture);
  //   glUseProgram(program);
  
  //   glBegin(GL_QUADS);
  //   {
  //     glTexCoord2f(0, 0);
  //     glVertex2f(-1.0f, 1.0f);

  //     glTexCoord2f(1.0f, 0);
  //     glVertex2f(1.0f, 1.0f);

  //     glTexCoord2f(1.0f, 1.0f);
  //     glVertex2f(1.0f, -1.0f);

  //     glTexCoord2f(0, 1.0f);
  //     glVertex2f(-1.0f, -1.0f);
  //   }
  //   glEnd();
  // } else {
  //   glClearColor(0, 0, 0, 1);
  //   glClear(GL_COLOR_BUFFER_BIT);
  // }

  if (buffer != nullptr) {
    ogl_video_device.SetDefaultFBO(defaultFramebufferObject());
    ogl_video_device.Draw(buffer);
  }
}

void Screen::resizeGL(int width, int height) {
  auto dpr = devicePixelRatio();
  width  *= dpr;
  height *= dpr;

  // viewport_width = height + height / 2;
  // viewport_height = height;
  // viewport_x = (width - viewport_width) / 2;

  auto view_width  = height + height / 2;
  auto view_height = height;
  auto view_x = (width - view_width) / 2;

  ogl_video_device.SetViewport(view_x, 0, view_width, view_height);
}
