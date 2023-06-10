/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QCheckBox>
#include <QImage>
#include <QLabel>
#include <QPainter>

#include "widget/palette_box.hpp"

struct BackgroundViewer : QWidget {
  BackgroundViewer(nba::CoreBase* core, QWidget* parent = nullptr);

  void SetBackgroundID(int id);
  void Update();

  bool eventFilter(QObject* object, QEvent* event);

private:
  void DrawBackground();
  void DrawBackgroundMode0();
  void DrawBackgroundMode2();
  void DrawBackgroundMode3();
  void DrawBackgroundMode4();
  void DrawBackgroundMode5();

  void PresentBackground();

  nba::CoreBase* core;
  u16* pram;
  u8* vram;

  int bg_id = 0;

  QLabel* bg_mode_label;
  QLabel* bg_priority_label;
  QLabel* bg_size_label;
  QLabel* bg_tile_base_label;
  QLabel* bg_map_base_label;
  QCheckBox* bg_8bpp_check_box;
  QCheckBox* bg_wraparound_check_box;
  QCheckBox* bg_mosaic_check_box;
  QLabel* bg_scroll_label;

  PaletteBox* tile_box;
  
  QWidget* canvas;
  QImage image{1024, 1024, QImage::Format_RGB555};

  Q_OBJECT
};