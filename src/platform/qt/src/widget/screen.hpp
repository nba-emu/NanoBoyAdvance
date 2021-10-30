/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <platform/device/ogl_video_device.hpp>
#include <QGLWidget>
#include <QOpenGLWidget>

struct Screen : QOpenGLWidget, nba::VideoDevice {
  Screen(
    QWidget* parent,
    std::shared_ptr<nba::PlatformConfig> config
  );

  void Draw(u32* buffer) final;
  void Clear();
  void ReloadConfig();

signals:
  void RequestDraw(u32* buffer);

private slots:
  void OnRequestDraw(u32* buffer);

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

private:
  static constexpr int kGBANativeWidth = 240;
  static constexpr int kGBANativeHeight = 160;
  static constexpr float kGBANativeAR = static_cast<float>(kGBANativeWidth) / static_cast<float>(kGBANativeHeight);

  u32* buffer = nullptr;
  bool should_clear = false;
  nba::OGLVideoDevice ogl_video_device;

  Q_OBJECT
};
