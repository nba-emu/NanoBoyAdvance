/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/device/video_device.hpp>
#include <QGLWidget>
#include <QOpenGLWidget>

struct Screen : QOpenGLWidget, nba::VideoDevice {
  Screen(QWidget* parent);
 ~Screen() override;

  void Draw(u32* buffer) final;

signals:
  void RequestDraw(u32* buffer);

private slots:
  void OnRequestDraw(u32* buffer);

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;


private:
  auto CreateShader() -> GLuint;

  int viewport_x = 0;
  int viewport_width = 0;
  int viewport_height = 0;

  GLuint texture;
  GLuint program;

  Q_OBJECT
};
