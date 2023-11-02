/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QCheckBox>
#include <QFontDatabase>
#include <QImage>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QSpinBox>
#include <QWidget>

struct SpriteViewer : QWidget {
  SpriteViewer(nba::CoreBase* core, QWidget* parent = nullptr);

  bool eventFilter(QObject* object, QEvent* event) override;

  void Update();

private:
  void RenderSprite(int index, u32* buffer, int stride);
  void RenderSpriteAtlas();

  nba::CoreBase* core;
  u16* pram;
  u8* vram;
  u8* oam;

  QImage image_rgb32{64, 64, QImage::Format_RGB32};
  QSpinBox* sprite_index_input;
  QSpinBox* magnification_input;
  QWidget* canvas;

  QImage atlas_image_rgb32{1024, 512, QImage::Format_RGB32};
  QWidget* atlas_canvas;

  QCheckBox* check_sprite_enabled;
  QLabel* label_sprite_position;
  QLabel* label_sprite_size;
  QLabel* label_sprite_tile_number;
  QLabel* label_sprite_palette;
  QCheckBox* check_sprite_8bpp;
  QCheckBox* check_sprite_vflip;
  QCheckBox* check_sprite_hflip;
  QLabel* label_sprite_mode;
  QCheckBox* check_sprite_affine;
  QLabel* label_sprite_transform;
  QCheckBox* check_sprite_double_size;
  QCheckBox* check_sprite_mosaic;
  QLabel* label_sprite_render_cycles;

  int sprite_width = 0;
  int sprite_height = 0;
  int magnified_sprite_width = 0;
  int magnified_sprite_height = 0;

  Q_OBJECT
};
