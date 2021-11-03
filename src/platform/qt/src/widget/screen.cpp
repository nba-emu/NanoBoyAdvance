/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <GL/glew.h>

#include "widget/screen.hpp"

Screen::Screen(
  QWidget* parent,
  std::shared_ptr<nba::PlatformConfig> config
)   : QOpenGLWidget(parent)
    , ogl_video_device(config) {
  QSurfaceFormat format;
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(3);
  format.setMinorVersion(3);
  format.setSwapInterval(0);
  setFormat(format);
  connect(this, &Screen::RequestDraw, this, &Screen::OnRequestDraw);
}

void Screen::Draw(u32* buffer) {
  should_clear = false;
  emit RequestDraw(buffer);
}

void Screen::Clear() {
  // TODO: this no longer works at this point.
  should_clear = true;
  update();
}

void Screen::ReloadConfig() {
  ogl_video_device.ReloadConfig();
}

void Screen::OnRequestDraw(u32* buffer) {
  this->buffer = buffer;
  update();
}

void Screen::initializeGL() {
  makeCurrent();
  ogl_video_device.Initialize();
}

void Screen::paintGL() {
  if (buffer != nullptr) {
    ogl_video_device.SetDefaultFBO(defaultFramebufferObject());
    ogl_video_device.Draw(buffer);
  }
}

void Screen::resizeGL(int width, int height) {
  auto dpr = devicePixelRatio();
  width  *= dpr;
  height *= dpr;

  int viewport_width;
  int viewport_height;
  int viewport_x;
  int viewport_y;

  float ar = static_cast<float>(width) / static_cast<float>(height);

  if (ar > kGBANativeAR) {
    viewport_width = static_cast<int>(height * kGBANativeAR);
    viewport_height = height;
    viewport_x = (width - viewport_width) / 2;
    viewport_y = 0;
  } else {
    viewport_width = width;
    viewport_height = static_cast<int>(width / kGBANativeAR);
    viewport_x = 0;
    viewport_y = (height - viewport_height) / 2;
  }

  ogl_video_device.SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
}
