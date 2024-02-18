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

class ColorGrid : public QWidget {
  public:
    ColorGrid(int rows, int columns, QWidget* parent = nullptr);
   ~ColorGrid() override; 

    void Draw(u16* buffer_rgb565, int stride);
    void SetHighlightedPosition(int x, int y);
    void ClearHighlight();
    u32  GetColorAt(int x, int y);

  signals:
    void selected(int x, int y);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

  private:
    static constexpr int k_box_size = 12;

    int m_rows;
    int m_columns;
    int m_highlighted_x = -1;
    int m_highlighted_y = -1;

    u32* m_buffer_argb8888;

    Q_OBJECT
};