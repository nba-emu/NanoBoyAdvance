/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <QGLWidget>
#include <QOpenGLWidget>
#include <emulator/device/video_device.hpp>

class Screen : public QOpenGLWidget,
               public nba::VideoDevice {
  Q_OBJECT

public:
  Screen(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat format;
    format.setSwapInterval(0);
    setFormat(format);

    // The signal-slot structure is needed, so that Screen::Draw
    // may be invoked from another thread without causing trouble.
    connect(this, &Screen::SignalDraw, this, &Screen::DrawSlot);
  }

  ~Screen() override { glDeleteTextures(1, &texture); }

  void Draw(std::uint32_t* buffer) final;

  auto sizeHint() const -> QSize {
    return QSize{ 480, 320 };
  }

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

private slots:
          void DrawSlot(std::uint32_t* buffer);

  signals:
          void SignalDraw(std::uint32_t* buffer);

private:
  auto CompileShader() -> GLuint;

  int viewport_x = 0;
  int viewport_width = 0;
  int viewport_height = 0;

  GLuint texture;
  GLuint program;
};
