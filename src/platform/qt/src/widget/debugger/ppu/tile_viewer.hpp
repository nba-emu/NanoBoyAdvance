/*
 * Copyright (C) 2025 fleroviux
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
#include "color_grid.hpp"

class TileViewer : public QWidget {
  public:
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

    u16* m_image_rgb565;
    QImage m_image_rgb32{256, 256, QImage::Format_RGB32};
    QSpinBox* m_spin_magnification;
    QSpinBox* m_spin_palette_index;
    QCheckBox* m_check_eight_bpp;
    QLabel* m_label_tile_number;
    QLabel* m_label_tile_address;
    ColorGrid* m_tile_color_grid;
    QLabel* m_label_color_r_component;
    QLabel* m_label_color_g_component;
    QLabel* m_label_color_b_component;
    QWidget* m_canvas;

    u32 m_tile_base = 0;
    bool m_display_selected_tile = false;
    int m_selected_tile_x;
    int m_selected_tile_y;

    u8*  m_vram;
    u16* m_pram;

    Q_OBJECT
};
