/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>
#include <QPainter>

#include "palette_box.hpp"

PaletteBox::PaletteBox(QWidget* parent) : QWidget(parent) {
  std::memset(palette_argb8888, 0, sizeof(palette_argb8888));

  setFixedSize(16 * k_box_size, 16 * k_box_size);
}

void PaletteBox::Draw(u16* palette_rgb565) {
  for(int i = 0; i < 256; i++) {
    const u16 color_rgb565 = palette_rgb565[i];

    const int r =  (color_rgb565 >>  0) & 31;
    const int g = ((color_rgb565 >>  4) & 62) | (color_rgb565 >> 15);
    const int b =  (color_rgb565 >> 10) & 31;

    const int x = i & 15;
    const int y = i >> 4;

    palette_argb8888[x][y] = 0xFF000000 |
                             (r << 3 | r >> 2) << 16 |
                             (g << 2 | g >> 4) <<  8 |
                             (b << 3 | b >> 2); 
  }

  update();
}

void PaletteBox::SetHighlightedPosition(int x, int y) {
  highlighted_x = x;
  highlighted_y = y;
  update();
}

void PaletteBox::ClearHighlight() {
  SetHighlightedPosition(-1, -1);
}

void PaletteBox::paintEvent(QPaintEvent* event) {
  QPainter painter{this};

  painter.setPen(Qt::gray);

  for(int y = 0; y < 16; y++) {
    for(int x = 0; x < 16; x++) {
      painter.setBrush(QBrush{palette_argb8888[x][y]});
      painter.drawRect(x * k_box_size, y * k_box_size, k_box_size, k_box_size);
    }
  }

  if(highlighted_x > -1 && highlighted_y > -1) {
    QPen red_highlight_pen{Qt::red};
    red_highlight_pen.setWidth(2);

    painter.setPen(red_highlight_pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(highlighted_x * k_box_size, highlighted_y * k_box_size, k_box_size, k_box_size);
  }
}

void PaletteBox::mousePressEvent(QMouseEvent* event) {
  const int x = (int)(event->x() / k_box_size);
  const int y = (int)(event->y() / k_box_size);

  emit selected(x, y);
}