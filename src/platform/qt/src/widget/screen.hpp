/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <platform/device/ogl_video_device.hpp>
#include <QOpenGLContext>
#include <QWidget>

#include "config.hpp"

struct Screen : QWidget, nba::VideoDevice {
  explicit Screen(
    QWidget* parent,
    std::shared_ptr<QtConfig> config
  );

  bool Initialize();
  void Draw(u32* buffer) final;
  void ReloadConfig();
  QPaintEngine* paintEngine() const override { return nullptr; }; // Silence Qt.

signals:
  void RequestDraw(u32* buffer);

private slots:
  void OnRequestDraw(u32* buffer);

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  static constexpr int kGBANativeWidth = 240;
  static constexpr int kGBANativeHeight = 160;
  static constexpr float kGBANativeAR = static_cast<float>(kGBANativeWidth) / static_cast<float>(kGBANativeHeight);

  void Render();
  void UpdateViewport();

  u32* buffer = nullptr;
  QOpenGLContext* context = nullptr;
  nba::OGLVideoDevice ogl_video_device;
  std::shared_ptr<QtConfig> config;

  Q_OBJECT
};
