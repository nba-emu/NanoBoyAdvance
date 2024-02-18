/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <fmt/format.h>
#include <nba/common/punning.hpp>

#include <QGridLayout>
#include <QVBoxLayout>

#include "widget/debugger/utility.hpp"
#include "sprite_viewer.hpp"

SpriteViewer::SpriteViewer(nba::CoreBase* core, QWidget* parent) : QWidget{parent}, m_core{core} {
  QHBoxLayout* hbox = new QHBoxLayout{};

  hbox->addLayout(CreateInfoLayout());
  hbox->addLayout(CreateCanvasLayout());
  hbox->addStretch(1);
  setLayout(hbox);

  m_pram = (u16*)core->GetPRAM();
  m_vram = core->GetVRAM();
  m_oam  = core->GetOAM();
}

QWidget* SpriteViewer::CreateSpriteIndexInput() {
  m_spin_sprite_index = new QSpinBox{};
  m_spin_sprite_index->setMinimum(0);
  m_spin_sprite_index->setMaximum(127);
  return m_spin_sprite_index;
}

QWidget* SpriteViewer::CreateMagnificationInput() {
  m_spin_magnification = new QSpinBox{};
  m_spin_magnification->setMinimum(1);
  m_spin_magnification->setMaximum(16);
  connect(m_spin_magnification, QOverload<int>::of(&QSpinBox::valueChanged), [this](int _) { Update(); });
  return m_spin_magnification;
}

QGroupBox* SpriteViewer::CreateSpriteAttributesGroupBox() {
  QGridLayout* grid = new QGridLayout{};

  QGroupBox* group_box = new QGroupBox{};
  group_box->setTitle("Object Attributes");
  group_box->setLayout(grid);
  
  m_check_sprite_enabled = CreateReadOnlyCheckBox();
  m_label_sprite_position = CreateMonospaceLabel();
  m_label_sprite_size = CreateMonospaceLabel();
  m_label_sprite_tile_number = CreateMonospaceLabel();
  m_label_sprite_palette = CreateMonospaceLabel();
  m_check_sprite_8bpp = CreateReadOnlyCheckBox();
  m_check_sprite_vflip = CreateReadOnlyCheckBox();
  m_check_sprite_hflip = CreateReadOnlyCheckBox();
  m_label_sprite_mode = CreateMonospaceLabel();
  m_check_sprite_affine = CreateReadOnlyCheckBox();
  m_label_sprite_transform = CreateMonospaceLabel();
  m_check_sprite_double_size = CreateReadOnlyCheckBox();
  m_check_sprite_mosaic = CreateReadOnlyCheckBox();
  m_label_sprite_render_cycles = CreateMonospaceLabel();

  int row = 0;

  const auto PushRow = [&](const QString &label, QWidget *widget) {
    grid->addWidget(new QLabel{QStringLiteral("%1:").arg(label)}, row, 0);
    grid->addWidget(widget, row++, 1);
  };

  PushRow("Enabled", m_check_sprite_enabled);
  PushRow("Position", m_label_sprite_position);
  PushRow("Size", m_label_sprite_size);
  PushRow("Tile #", m_label_sprite_tile_number);
  PushRow("Palette", m_label_sprite_palette);
  PushRow("8BPP", m_check_sprite_8bpp);
  PushRow("Flip V", m_check_sprite_vflip);
  PushRow("Flip H", m_check_sprite_hflip);
  PushRow("Mode", m_label_sprite_mode);
  PushRow("Affine", m_check_sprite_affine);
  PushRow("Transform #", m_label_sprite_transform);
  PushRow("Double-size", m_check_sprite_double_size);
  PushRow("Mosaic", m_check_sprite_mosaic);
  PushRow("Render cycles", m_label_sprite_render_cycles);
  return group_box;
}

QLayout* SpriteViewer::CreateInfoLayout() {
  QVBoxLayout* vbox = new QVBoxLayout{};
  QGridLayout* grid = new QGridLayout{};

  int row = 0;
  grid->addWidget(new QLabel(tr("OAM #:")), row, 0);
  grid->addWidget(CreateSpriteIndexInput(), row++, 1);
  grid->addWidget(new QLabel(tr("Magnification:")), row, 0);
  grid->addWidget(CreateMagnificationInput(), row++, 1);

  vbox->addLayout(grid);
  vbox->addWidget(CreateSpriteAttributesGroupBox());
  vbox->addStretch();
  return vbox;
}

QLayout* SpriteViewer::CreateCanvasLayout() {
  QVBoxLayout* vbox = new QVBoxLayout{};

  m_canvas = new QWidget{};
  m_canvas->setFixedSize(64, 64);
  m_canvas->setFixedSize(64, 64);
  m_canvas->installEventFilter(this);
  vbox->addWidget(m_canvas);
  vbox->addStretch(1);
  return vbox;
}

void SpriteViewer::Update() {
  const int offset = m_spin_sprite_index->value() << 3;

  const u16 attr0 = nba::read<u16>(m_oam, offset);
  const u16 attr1 = nba::read<u16>(m_oam, offset + 2);
  const u16 attr2 = nba::read<u16>(m_oam, offset + 4);

  const int shape = attr0 >> 14;
  const int size  = attr1 >> 14;

  static constexpr int k_sprite_size[4][4][2] = {
    { { 8 , 8  }, { 16, 16 }, { 32, 32 }, { 64, 64 } }, // Square
    { { 16, 8  }, { 32, 8  }, { 32, 16 }, { 64, 32 } }, // Horizontal
    { { 8 , 16 }, { 8 , 32 }, { 16, 32 }, { 32, 64 } }, // Vertical
    { { 8 , 8  }, { 8 , 8  }, { 8 , 8  }, { 8 , 8  } }  // Prohibited
  };

  const int width = k_sprite_size[shape][size][0];
  const int height = k_sprite_size[shape][size][1];

  const bool is_8bpp = attr0 & (1 << 13);
  const uint tile_number = attr2 & 0x3FFu;

  const bool one_dimensional_mapping = m_core->PeekHalfIO(0x04000000) & (1 << 6);

  const int tiles_x = width >> 3;
  const int tiles_y = height >> 3;

  u32* const buffer = (u32*)m_image_rgb32.bits();

  if(is_8bpp) {
    const u16* palette = &m_pram[256];

    for(int tile_y = 0; tile_y < tiles_y; tile_y++) {
      for(int tile_x = 0; tile_x < tiles_x; tile_x++) {
        int current_tile_number;

        if(one_dimensional_mapping) {
          current_tile_number = (tile_number + tile_y * (width >> 2) + (tile_x << 1)) & 0x3FF;
        } else {
          current_tile_number = ((tile_number + (tile_y * 32)) & 0x3E0) | (((tile_number & ~1) + (tile_x << 1)) & 0x1F);
        }

        // @todo: handle bad tile numbers and overflows and the likes

        u32 tile_address = 0x10000u + (current_tile_number << 5);

        for(int y = 0; y < 8; y++) {
          u64 tile_data = nba::read<u64>(m_vram, tile_address);

          u32* dst = &buffer[tile_y << 9 | y << 6 | tile_x << 3];

          for(int x = 0; x < 8; x++) {
            *dst++ = Rgb565ToArgb8888(palette[tile_data & 255u]);
            tile_data >>= 8;
          }

          tile_address += sizeof(u64);
        }
      }
    }
  } else {
    const u16* palette = &m_pram[256 | attr2 >> 12 << 4];

    for(int tile_y = 0; tile_y < tiles_y; tile_y++) {
      for(int tile_x = 0; tile_x < tiles_x; tile_x++) {
        int current_tile_number;

        if(one_dimensional_mapping) {
          current_tile_number = (tile_number + tile_y * (width >> 3) + tile_x) & 0x3FF;
        } else {
          current_tile_number = ((tile_number + (tile_y * 32)) & 0x3E0) | ((tile_number + tile_x) & 0x1F);
        }

        // @todo: handle bad tile numbers and overflows and the likes

        u32 tile_address = 0x10000u + (current_tile_number << 5);

        for(int y = 0; y < 8; y++) {
          u32 tile_data = nba::read<u32>(m_vram, tile_address);

          u32* dst = &buffer[tile_y << 9 | y << 6 | tile_x << 3];

          for(int x = 0; x < 8; x++) {
            *dst++ = Rgb565ToArgb8888(palette[tile_data & 15u]);
            tile_data >>= 4;
          }

          tile_address += sizeof(u32);
        }
      }
    }
  }

  static constexpr const char* k_mode_names[4] {
    "Normal", "Semi-Transparent", "Window", "Prohibited"
  };

  const uint x = attr1 & 0x1FFu;
  const uint y = attr0 & 0x0FFu;
  const bool affine = attr0 & (1 << 8);
  const uint palette = is_8bpp ? 0u : (attr2 >> 12);
  const uint mode = (attr0 >> 10) & 3;
  const bool mosaic = attr0 & (1 << 12);

  m_label_sprite_position->setText(QStringLiteral("%1, %2").arg(x).arg(y));
  m_label_sprite_size->setText(QStringLiteral("%1, %2").arg(width).arg(height));
  m_label_sprite_tile_number->setText(QStringLiteral("%1").arg(tile_number));
  m_label_sprite_palette->setText(QStringLiteral("%1").arg(palette));
  m_check_sprite_8bpp->setChecked(is_8bpp);
  m_label_sprite_mode->setText(k_mode_names[mode]);
  m_check_sprite_affine->setChecked(affine);
  m_check_sprite_mosaic->setChecked(mosaic);

  m_check_sprite_vflip->setEnabled(!affine);
  m_check_sprite_hflip->setEnabled(!affine);
  m_check_sprite_double_size->setEnabled(affine);

  const int signed_x = x >= 240 ? ((int)x - 512) : (int)x;

  int render_cycles = 0;

  if(affine) {
    const bool double_size = attr0 & (1 << 9);
    const uint transform = (attr1 >> 9) & 31u;

    m_check_sprite_enabled->setChecked(true);
    m_check_sprite_vflip->setChecked(false);
    m_check_sprite_hflip->setChecked(false);
    m_label_sprite_transform->setText(QStringLiteral("%1").arg(transform));
    m_check_sprite_double_size->setChecked(double_size);
    m_label_sprite_render_cycles->setText("0 (0%)");

    const int clipped_draw_width = std::max(0, (double_size ? 2 * width : width) + std::min(signed_x, 0));

    render_cycles = 10 + 2 * clipped_draw_width;
  } else {
    const bool enabled = !(attr0 & (1 << 9));
    const bool hflip = attr1 & (1 << 12);
    const bool vflip = attr1 & (1 << 13);

    m_check_sprite_enabled->setChecked(enabled);
    m_check_sprite_vflip->setChecked(vflip);
    m_check_sprite_hflip->setChecked(hflip);
    m_label_sprite_transform->setText("n/a");
    m_check_sprite_double_size->setChecked(false);
    m_label_sprite_render_cycles->setText("0 (0%)");

    if(enabled) {
      render_cycles = std::max(0, width + std::min(signed_x, 0));
    }
  }

  const int available_render_cycles = (m_core->PeekHalfIO(0x04000000) & (1 << 5)) ? 964 : 1232;

  m_label_sprite_render_cycles->setText(QString::fromStdString(fmt::format("{} ({:.2f} %)", render_cycles, 100.0f * (float)render_cycles / available_render_cycles)));

  m_sprite_width = width;
  m_sprite_height = height;

  const int magnification = m_spin_magnification->value();
  m_magnified_sprite_width = width * magnification;
  m_magnified_sprite_height = height * magnification;
  m_canvas->setFixedSize(m_magnified_sprite_width, m_magnified_sprite_height);
  m_canvas->update();
}

bool SpriteViewer::eventFilter(QObject* object, QEvent* event) {
  if(object == m_canvas && event->type() == QEvent::Paint) {
    const QRect src_rect{0, 0, m_sprite_width, m_sprite_height};
    const QRect dst_rect{0, 0, m_magnified_sprite_width, m_magnified_sprite_height};

    QPainter painter{m_canvas};
    painter.drawImage(dst_rect, m_image_rgb32, src_rect);
    return true;
  }

  return false;
}