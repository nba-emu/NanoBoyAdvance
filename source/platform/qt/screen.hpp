/**
  * Copyright (C) 2020 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <QGLWidget>
#include <QOpenGLWidget>
#include <gba/config/device/video_device.hpp>

class Screen : public QOpenGLWidget,
               public GameBoyAdvance::VideoDevice {
  Q_OBJECT

public:
  Screen(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat format;
    format.setSwapInterval(0);
    setFormat(format);

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
  int viewport_x = 0;
  int viewport_width = 0;
  int viewport_height = 0;

  GLuint texture;
};