/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <cstring>
#include <QPainter>

#include "widget/debugger/utility.hpp"
#include "palette_box.hpp"

PaletteBox::PaletteBox(int rows, int columns, QWidget* parent)
    : QWidget(parent)
    , m_rows(rows)
    , m_columns(columns) {
  m_palette_argb8888 = new u32[rows * columns];
  std::memset(m_palette_argb8888, 0, rows * columns * sizeof(u32));

  setFixedSize(columns * k_box_size, rows * k_box_size);
}

PaletteBox::~PaletteBox() {
  delete[] m_palette_argb8888;
}

void PaletteBox::Draw(u16* palette_rgb565, int stride) {
  int i = 0;

  for(int y = 0; y < m_rows; y++) {
    for(int x = 0; x < m_columns; x++) {
      m_palette_argb8888[i++] = Rgb565ToArgb8888(palette_rgb565[y * stride + x]);
    }
  }

  update();
}

void PaletteBox::SetHighlightedPosition(int x, int y) {
  m_highlighted_x = std::min(m_columns, x);
  m_highlighted_y = std::min(m_rows, y);
  update();
}

void PaletteBox::ClearHighlight() {
  SetHighlightedPosition(-1, -1);
}

u32 PaletteBox::GetColorAt(int x, int y) {
  return m_palette_argb8888[y * m_columns + x];
}

void PaletteBox::paintEvent(QPaintEvent* event) {
  QPainter painter{this};

  painter.setPen(Qt::gray);

  int i = 0;

  for(int y = 0; y < m_rows; y++) {
    for(int x = 0; x < m_columns; x++) {
      painter.setBrush(QBrush{m_palette_argb8888[i++]});
      painter.drawRect(x * k_box_size, y * k_box_size, k_box_size, k_box_size);
    }
  }

  if(m_highlighted_x > -1 && m_highlighted_y > -1) {
    QPen red_highlight_pen{Qt::red};
    red_highlight_pen.setWidth(2);

    painter.setPen(red_highlight_pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_highlighted_x * k_box_size, m_highlighted_y * k_box_size, k_box_size, k_box_size);
  }
}

void PaletteBox::mousePressEvent(QMouseEvent* event) {
  const int x = std::min((int)(event->x() / k_box_size), m_columns);
  const int y = std::min((int)(event->y() / k_box_size), m_rows);

  emit selected(x, y);
}