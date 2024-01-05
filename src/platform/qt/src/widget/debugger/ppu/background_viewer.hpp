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

#include "palette_box.hpp"

struct BackgroundViewer : QWidget {
  BackgroundViewer(nba::CoreBase* core, QWidget* parent = nullptr);
 ~BackgroundViewer() override; 

  void SetBackgroundID(int id);
  void Update();

  bool eventFilter(QObject* object, QEvent* event);

private:
  void DrawBackgroundMode0();
  void DrawBackgroundMode2();
  void DrawBackgroundMode3();
  void DrawBackgroundMode4();
  void DrawBackgroundMode5();

  void DrawTileDetail(int tile_x, int tile_y);
  void ClearTileSelection();

  void PresentBackground();

  nba::CoreBase* core;
  u16* pram;
  u8* vram;

  int bg_id = 0;
  int bg_mode = 0;
  u16 bghofs = 0;
  u16 bgvofs = 0;

  QLabel* bg_mode_label;
  QLabel* bg_priority_label;
  QLabel* bg_size_label;
  QLabel* bg_tile_base_label;
  QLabel* bg_map_base_label;
  QCheckBox* bg_8bpp_check_box;
  QCheckBox* bg_wraparound_check_box;
  QCheckBox* bg_mosaic_check_box;
  QLabel* bg_scroll_label;

  QLabel* tile_number_label;
  QLabel* tile_address_label;
  QLabel* tile_map_entry_address_label;
  QCheckBox* tile_flip_v_check_box;
  QCheckBox* tile_flip_h_check_box;
  QLabel* tile_palette_label;
  PaletteBox* tile_box;
  QLabel* tile_color_r_component_label;
  QLabel* tile_color_g_component_label;
  QLabel* tile_color_b_component_label;

  QCheckBox* display_screen_viewport_check_box;

  bool display_selected_tile = false;
  int selected_tile_x;
  int selected_tile_y;
  
  QWidget* canvas;
  QImage image_rgb32{1024, 1024, QImage::Format_RGB32};
  u16* image_rgb565;

  struct TileMetaData {
    int tile_number;
    u32 tile_address;
    u32 map_entry_address;
    bool flip_v;
    bool flip_h;
    int palette;
  } tile_meta_data[128][128];

  Q_OBJECT
};