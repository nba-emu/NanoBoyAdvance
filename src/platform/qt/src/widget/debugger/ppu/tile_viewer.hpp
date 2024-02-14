/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QLabel>
#include <QPaintEvent>
#include <QSpinBox>
#include <QWidget>

struct TileViewer : QWidget {
  TileViewer(nba::CoreBase* core, QWidget* parent = nullptr);

  bool eventFilter(QObject* object, QEvent* event) override;

  void Update();

private:
  u8* vram;
  u16* pram;
  QImage image_rgb32{256, 256, QImage::Format_RGB32};
  QSpinBox* palette_input;
  QSpinBox* magnification_input;
  QWidget* canvas;

  u32 tile_base = 0;
  bool eight_bpp = false;
  //int palette_index = 0;

  Q_OBJECT
};
