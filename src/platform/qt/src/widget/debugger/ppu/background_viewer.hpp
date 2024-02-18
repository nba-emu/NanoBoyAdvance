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

#include "color_grid.hpp"

class BackgroundViewer : public QWidget {
  public:
    BackgroundViewer(nba::CoreBase* core, QWidget* parent = nullptr);
   ~BackgroundViewer() override; 

    void SetBackgroundID(int id);
    void Update();

    bool eventFilter(QObject* object, QEvent* event);

  private:
    QWidget* CreateBackgroundInfoGroupBox();
    QWidget* CreateTileInfoGroupBox();
    QLayout* CreateInfoPanel();
    QWidget* CreateCanvasScrollArea();

    void DrawBackgroundMode0();
    void DrawBackgroundMode2();
    void DrawBackgroundMode3();
    void DrawBackgroundMode4();
    void DrawBackgroundMode5();

    void DrawTileDetail(int tile_x, int tile_y);
    void ClearTileSelection();

    void PresentBackground();

    int m_bg_id = 0;
    int m_bg_mode = 0;
    u16 m_bghofs = 0;
    u16 m_bgvofs = 0;

    QLabel* m_label_bg_mode;
    QLabel* m_label_bg_priority;
    QLabel* m_label_bg_size;
    QLabel* m_label_tile_base;
    QLabel* m_label_map_base;
    QCheckBox* m_check_8bpp;
    QCheckBox* m_check_bg_wraparound;
    QCheckBox* m_check_bg_mosaic;
    QLabel* m_label_bg_scroll;

    QLabel* m_label_tile_number;
    QLabel* m_label_tile_address;
    QLabel* m_label_tile_map_entry_address;
    QCheckBox* m_check_tile_flip_v;
    QCheckBox* m_check_tile_flip_h;
    QLabel* m_label_tile_palette;
    ColorGrid* m_tile_color_grid;
    QLabel* m_label_color_r_component;
    QLabel* m_label_color_g_component;
    QLabel* m_label_color_b_component;

    QCheckBox* m_check_display_screen_viewport;

    bool m_display_selected_tile = false;
    int m_selected_tile_x;
    int m_selected_tile_y;
    
    QWidget* m_canvas;
    QImage m_image_rgb32{1024, 1024, QImage::Format_RGB32};
    u16* m_image_rgb565;

    struct TileMetaData {
      int tile_number;
      u32 tile_address;
      u32 map_entry_address;
      bool flip_v;
      bool flip_h;
      int palette;
    } m_tile_meta_data[128][128];

    nba::CoreBase* m_core;
    u16* m_pram;
    u8*  m_vram;

    Q_OBJECT
};