/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QLabel>

#include "color_grid.hpp"

class PaletteViewer : public QWidget {
  public:
    PaletteViewer(nba::CoreBase* core, QWidget* parent = nullptr);

    void Update();

  private:
    QLayout* CreatePaletteGrids();
    QLayout* CreateColorInfoArea();

    void ShowColorInfo(int color_index);

    u16* m_pram;
    ColorGrid* m_palette_color_grids[2];
    QLabel* m_label_color_address;
    QLabel* m_label_color_r_component;
    QLabel* m_label_color_g_component;
    QLabel* m_label_color_b_component;
    QLabel* m_label_color_value;
    QWidget* m_box_color_preview;

    Q_OBJECT
};
