/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QCheckBox>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QSpinBox>
#include <QWidget>

class SpriteViewer : public QWidget {
  public:
    SpriteViewer(nba::CoreBase* core, QWidget* parent = nullptr);

    void Update();
    bool eventFilter(QObject* object, QEvent* event) override;

  private:
    QWidget* CreateSpriteIndexInput();
    QWidget* CreateMagnificationInput();
    QGroupBox* CreateSpriteAttributesGroupBox();
    QLayout* CreateInfoLayout();
    QLayout* CreateCanvasLayout();

    QImage m_image_rgb32{64, 64, QImage::Format_RGB32};
    QSpinBox* m_spin_sprite_index;
    QSpinBox* m_spin_magnification;
    QWidget* m_canvas;

    QCheckBox* m_check_sprite_enabled;
    QLabel* m_label_sprite_position;
    QLabel* m_label_sprite_size;
    QLabel* m_label_sprite_tile_number;
    QLabel* m_label_sprite_palette;
    QCheckBox* m_check_sprite_8bpp;
    QCheckBox* m_check_sprite_vflip;
    QCheckBox* m_check_sprite_hflip;
    QLabel* m_label_sprite_mode;
    QCheckBox* m_check_sprite_affine;
    QLabel* m_label_sprite_transform;
    QCheckBox* m_check_sprite_double_size;
    QCheckBox* m_check_sprite_mosaic;
    QLabel* m_label_sprite_render_cycles;

    int m_sprite_width = 0;
    int m_sprite_height = 0;
    int m_magnified_sprite_width = 0;
    int m_magnified_sprite_height = 0;

    nba::CoreBase* m_core;
    u16* m_pram;
    u8* m_vram;
    u8* m_oam;

    Q_OBJECT
};
