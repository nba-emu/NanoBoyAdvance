/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <GL/glew.h>

#include "widget/screen.hpp"

Screen::Screen(
  QWidget* parent,
  std::shared_ptr<QtConfig> config
)   : QOpenGLWidget(parent)
    , ogl_video_device(config)
    , config(config) {
  connect(this, &Screen::RequestDraw, this, &Screen::OnRequestDraw);
}

void Screen::Draw(u32* buffer) {
  emit RequestDraw(buffer);
}

void Screen::SetForceClear(bool force_clear) {
  this->force_clear = force_clear;
  update();
}

void Screen::ReloadConfig() {
  ogl_video_device.ReloadConfig();
  UpdateViewport();
}

void Screen::OnRequestDraw(u32* buffer) {
  this->buffer = buffer;
  update();
}

void Screen::UpdateViewport() {
  auto dpr = devicePixelRatio();
  int width  = size().width()  * dpr;
  int height = size().height() * dpr;

  int viewport_width;
  int viewport_height;
  int viewport_x;
  int viewport_y;

  int max_scale = config->window.maximum_scale;

  viewport_width  = width;
  viewport_height = height;

  // Lock width and height to an aspect ratio of 3:2 (original GBA screen)
  if(config->window.lock_aspect_ratio) {
    float ar = static_cast<float>(width) / static_cast<float>(height);

    if(ar > kGBANativeAR) {
      viewport_width = static_cast<int>(height * kGBANativeAR);
    } else {
      viewport_height = static_cast<int>(width / kGBANativeAR);
    }
  }

  // Lock width and height to (non-uniform) integer scales of the native resolution
  if(config->window.use_integer_scaling) {
    viewport_width  = kGBANativeWidth  * std::max(1, static_cast<int>(viewport_width  / (float)kGBANativeWidth));
    viewport_height = kGBANativeHeight * std::max(1, static_cast<int>(viewport_height / (float)kGBANativeHeight));
  }

  // Limit screen size to a maximum scaling factor
  if(max_scale > 0) {
    int max_width  = kGBANativeWidth  * max_scale;
    int max_height = kGBANativeHeight * max_scale; 

    bool overflowing = viewport_width  >= max_width ||
                       viewport_height >= max_height;

    bool max_size_fits_into_window = width  >= max_width &&
                                     height >= max_height;

    if(overflowing && max_size_fits_into_window) {
      viewport_width  = max_width;
      viewport_height = max_height;
    }
  }

  // Center the viewport
  viewport_x = (width  - viewport_width ) / 2;
  viewport_y = (height - viewport_height) / 2;

  ogl_video_device.SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
}

void Screen::initializeGL() {
  makeCurrent();
  ogl_video_device.Initialize();
}

void Screen::paintGL() {
  if(force_clear) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
  } else if(buffer != nullptr) {
    ogl_video_device.SetDefaultFBO(defaultFramebufferObject());
    ogl_video_device.Draw(buffer);
  }
}

void Screen::resizeGL(int width, int height) {
  UpdateViewport();
}
