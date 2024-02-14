/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fmt/format.h>
#include <nba/common/punning.hpp>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QRadioButton>
#include <QVBoxLayout>

#include "widget/debugger/utility.hpp"
#include "tile_viewer.hpp"

TileViewer::TileViewer(nba::CoreBase* core, QWidget* parent) : QWidget(parent) {
  vram = core->GetVRAM();
  pram = (u16*)core->GetPRAM();

  QHBoxLayout* hbox = new QHBoxLayout{};

  {
    QVBoxLayout* vbox = new QVBoxLayout{};

    magnification_input = new QSpinBox{};
    magnification_input->setMinimum(1);
    magnification_input->setMaximum(16);
    vbox->addWidget(magnification_input);

    palette_input = new QSpinBox{};
    palette_input->setMinimum(0);
    palette_input->setMaximum(15);
    vbox->addWidget(palette_input);

    QGroupBox* tile_base_group_box = new QGroupBox{};
    tile_base_group_box->setTitle(tr("Tile Base"));

    QVBoxLayout* tile_base_vbox = new QVBoxLayout{};

    for(u32 tile_base = 0x06000000u; tile_base <= 0x06010000u; tile_base += 0x4000u) {
      QRadioButton* radio_button = new QRadioButton{
        QString::fromStdString(fmt::format("0x{:08X}", tile_base))};

      connect(radio_button, &QRadioButton::pressed, [this, tile_base]() {
        this->tile_base = tile_base & 0xFFFFFFu;
      });

      tile_base_vbox->addWidget(radio_button);
      
      if(tile_base == 0x06000000u) radio_button->click();
    }

    tile_base_group_box->setLayout(tile_base_vbox);

    vbox->addWidget(tile_base_group_box);

    hbox->addLayout(vbox);
  }

  canvas = new QWidget{};
  canvas->installEventFilter(this);
  hbox->addWidget(canvas);

  setLayout(hbox);
}

bool TileViewer::eventFilter(QObject* object, QEvent* event) {
  if(object == canvas && event->type() == QEvent::Paint) {
    const int canvas_w = canvas->size().width();
    const int canvas_h = canvas->size().height();

    const QRect src_rect{0, 0, 256, 256 * canvas_h / canvas_w};
    const QRect dst_rect{0, 0, canvas_w, canvas_h};

    QPainter painter{canvas};
    painter.drawImage(dst_rect, image_rgb32, src_rect);
    return true;
  }

  return false;
}

void TileViewer::Update() {
  if(!isVisible()) {
    return;
  }

  const int magnification = magnification_input->value();
  const int palette_offset = tile_base == 0x10000u ? 256 : 0;

  u32* const buffer = (u32*)image_rgb32.bits();

  int height = 256;
  u32 tile_address = tile_base;

  if(eight_bpp) {
    u16* const palette = &pram[palette_offset];

    for(int tile = 0; tile < 512; tile++) {
      const int tile_base_x = (tile % 32) * 8;
      const int tile_base_y = (tile / 32) * 8;

      for(int y = 0; y < 8; y++) {
        u64 tile_row_data = nba::read<u64>(vram, tile_address);

        for(int x = 0; x < 8; x++) {
          buffer[(tile_base_y + y) * 256 + tile_base_x + x] = Rgb565ToArgb8888(palette[(u8)tile_row_data]);
          tile_row_data >>= 8;
        }

        tile_address += sizeof(u64);
      }
    }

    height /= 2;
  } else {
    u16* const palette = &pram[palette_input->value() * 16 + palette_offset];

    for(int tile = 0; tile < 1024; tile++) {
      const int tile_base_x = (tile % 32) * 8;
      const int tile_base_y = (tile / 32) * 8;

      for(int y = 0; y < 8; y++) {
        u32 tile_row_data = nba::read<u32>(vram, tile_address);

        for(int x = 0; x < 8; x++) {
          buffer[(tile_base_y + y) * 256 + tile_base_x + x] = Rgb565ToArgb8888(palette[tile_row_data & 15]);
          tile_row_data >>= 4;
        }

        tile_address += sizeof(u32);
      }
    }
  }

  if(tile_base == 0xC000u) {
    height /= 2;
  }

  canvas->setFixedSize(256 * magnification, height * magnification);
  canvas->update();
}
