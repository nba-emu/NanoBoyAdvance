/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QCheckBox>
#include <QLabel>
#include <QPaintEvent>
#include <QSpinBox>
#include <QWidget>

#include "widget/debugger/utility.hpp"
#include "palette_box.hpp"

struct TileViewer : QWidget {
  TileViewer(nba::CoreBase* core, QWidget* parent = nullptr);
 ~TileViewer(); 

  bool eventFilter(QObject* object, QEvent* event) override;

  void Update();

private:
  QWidget* CreateMagnificationInput();
  QWidget* CreatePaletteInput();
  QWidget* CreateTileBaseGroupBox();
  QWidget* CreateTileInfoGroupBox();

  void PresentTileMap();
  void DrawTileDetail(int tile_x, int tile_y);
  void ClearTileSelection();
  void UpdateImpl();

  u8* vram;
  u16* pram;
  u16* image_rgb565;
  QImage image_rgb32{256, 256, QImage::Format_RGB32};
  QSpinBox* magnification_input;
  QSpinBox* palette_input;
  QCheckBox* eight_bpp;
  QLabel* label_tile_number;
  QLabel* label_tile_address;
  PaletteBox* tile_box;
  QLabel* label_color_r_component;
  QLabel* label_color_g_component;
  QLabel* label_color_b_component;
  QWidget* canvas;

  u32 tile_base = 0;
  bool display_selected_tile = false;
  int selected_tile_x;
  int selected_tile_y;

  Q_OBJECT
};
