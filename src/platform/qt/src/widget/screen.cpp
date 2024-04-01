/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "widget/screen.hpp"

#include <glad/gl.h>
#include <QOpenGLContext> // Has to go after glad.
#include <QWindow>

Screen::Screen(
  QWidget* parent,
  std::shared_ptr<QtConfig> config
)   : QWidget(parent)
    , ogl_video_device(config)
    , config(config) {
  connect(this, &Screen::RequestDraw, this, &Screen::OnRequestDraw);
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_PaintOnScreen);
  windowHandle()->setSurfaceType(QWindow::OpenGLSurface);
}

static auto get_proc_address(const char* proc_name) {
  return QOpenGLContext::currentContext()->getProcAddress(proc_name);
}

bool Screen::Initialize() {
  context = new QOpenGLContext();
  if(!context->create()) {
    delete context;
    context = nullptr;
    return false;
  }
  if(context->format().majorVersion() < 3 ||
     context->format().majorVersion() == 3 && context->format().minorVersion() < 3) {
    delete context;
    context = nullptr;
    return false;
  }

  if(!context->makeCurrent(this->windowHandle())) {
    delete context;
    context = nullptr;
    return false;
  }

  if(!gladLoadGL(get_proc_address)) {
    delete context;
    context = nullptr;
    return false;
  }

  ogl_video_device.Initialize();
  UpdateViewport();

  context->doneCurrent();

  return true;
}

void Screen::Draw(u32* buffer) {
  emit RequestDraw(buffer);
}

void Screen::ReloadConfig() {
  context->makeCurrent(this->windowHandle());
  ogl_video_device.ReloadConfig();
  UpdateViewport();
}

void Screen::OnRequestDraw(u32* buffer) {
  this->buffer = buffer;
  update();
}

void Screen::paintEvent([[maybe_unused]] QPaintEvent* event) {
  Render();
}

void Screen::resizeEvent([[maybe_unused]] QResizeEvent* event) {
  UpdateViewport();
}

void Screen::Render() {
  if(!context) {
    return;
  }

  context->makeCurrent(this->windowHandle());
  glClear(GL_COLOR_BUFFER_BIT);

  if(buffer) {
    ogl_video_device.SetDefaultFBO(context->defaultFramebufferObject());
    ogl_video_device.Draw(buffer);
  }

  context->swapBuffers(this->windowHandle());
  context->doneCurrent();
}

void Screen::UpdateViewport() {
  if(!context) {
    return;
  }

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

  context->makeCurrent(this->windowHandle());
  ogl_video_device.SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
  context->doneCurrent();
}
