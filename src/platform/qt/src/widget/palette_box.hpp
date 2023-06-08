/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

struct PaletteBox : QWidget {
  
  PaletteBox(QWidget* parent = nullptr);

  void Draw(u16* palette_rgb565);
  void SetHighlightedPosition(int x, int y);
  void ClearHighlight();

signals:
  void selected(int x, int y);

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

private:
  static constexpr int k_box_size = 12;

  int highlighted_x = -1;
  int highlighted_y = -1;

  u32 palette_argb8888[16][16];

  Q_OBJECT
};