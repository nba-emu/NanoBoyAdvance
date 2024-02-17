/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fmt/format.h>
#include <nba/common/punning.hpp>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QRadioButton>
#include <QVBoxLayout>

#include "tile_viewer.hpp"

TileViewer::TileViewer(nba::CoreBase* core, QWidget* parent) : QWidget(parent) {
  QHBoxLayout* hbox = new QHBoxLayout{};
  QVBoxLayout* vbox_l = new QVBoxLayout{};
  QVBoxLayout* vbox_r = new QVBoxLayout{};
  hbox->addLayout(vbox_l);
  hbox->addLayout(vbox_r);
  hbox->addStretch();
  setLayout(hbox);

  magnification_input = new QSpinBox{};
  magnification_input->setMinimum(1);
  magnification_input->setMaximum(16);
  connect(magnification_input, QOverload<int>::of(&QSpinBox::valueChanged), [this](int _) { Update(); });

  palette_input = new QSpinBox{};
  palette_input->setMinimum(0);
  palette_input->setMaximum(15);

  QGridLayout* grid = new QGridLayout{};
  grid->addWidget(new QLabel{tr("Magnification:")}, 0, 0);
  grid->addWidget(magnification_input, 0, 1);
  grid->addWidget(new QLabel{tr("Palette #:")}, 1, 0);
  grid->addWidget(palette_input, 1, 1);

  vbox_l->addLayout(grid);

  eight_bpp = new QCheckBox{tr("256 color mode (8BPP)")};
  vbox_l->addWidget(eight_bpp);

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

  vbox_l->addWidget(tile_base_group_box);

  QGroupBox* tile_data_group_box = new QGroupBox{};
  tile_data_group_box->setTitle(tr("Tile"));

  QVBoxLayout* tile_data_vbox = new QVBoxLayout{};

  QGridLayout* tile_data_grid = new QGridLayout{};
  label_tile_number  = CreateMonospaceLabel();
  label_tile_address = CreateMonospaceLabel();
  tile_data_grid->addWidget(new QLabel{tr("Tile #:")}, 0, 0);
  tile_data_grid->addWidget(label_tile_number, 0, 1);
  tile_data_grid->addWidget(new QLabel{tr("Tile address:")}, 1, 0);
  tile_data_grid->addWidget(label_tile_address, 1, 1);
  tile_data_vbox->addLayout(tile_data_grid);

  tile_box = new PaletteBox{8, 8};
  tile_data_vbox->addWidget(tile_box);
  tile_data_group_box->setLayout(tile_data_vbox);

  QGridLayout* color_rgb_grid = new QGridLayout{};
  label_color_r_component = CreateMonospaceLabel();
  label_color_g_component = CreateMonospaceLabel();
  label_color_b_component = CreateMonospaceLabel();
  color_rgb_grid->addWidget(new QLabel{tr("R:")}, 0, 0);
  color_rgb_grid->addWidget(label_color_r_component, 0, 1);
  color_rgb_grid->addWidget(new QLabel{tr("G:")}, 1, 0);
  color_rgb_grid->addWidget(label_color_g_component, 1, 1);
  color_rgb_grid->addWidget(new QLabel{tr("B:")}, 2, 0);
  color_rgb_grid->addWidget(label_color_b_component, 2, 1);
  tile_data_vbox->addLayout(color_rgb_grid);

  connect(tile_box, &PaletteBox::selected, [this](int x, int y) {
    const u32 color_rgb32 = tile_box->GetColorAt(x, y);
    const int r = (color_rgb32 >> 18) & 62;
    const int g = (color_rgb32 >> 10) & 63;
    const int b = (color_rgb32 >>  2) & 62;

    label_color_r_component->setText(QStringLiteral("%1").arg(r));
    label_color_g_component->setText(QStringLiteral("%1").arg(g));
    label_color_b_component->setText(QStringLiteral("%1").arg(b)); 

    tile_box->SetHighlightedPosition(x, y);
  });

  vbox_l->addWidget(tile_data_group_box);

  vbox_l->addStretch();

  canvas = new QWidget{};
  canvas->installEventFilter(this);
  vbox_r->addWidget(canvas);
  vbox_r->addStretch();

  vram = core->GetVRAM();
  pram = (u16*)core->GetPRAM();
  image_rgb565 = new u16[256 * 256];

  UpdateImpl();
}

TileViewer::~TileViewer() {
  delete image_rgb565;
}

bool TileViewer::eventFilter(QObject* object, QEvent* event) {
  if(object != canvas) {
    return false;
  }

  switch(event->type()) {
    case QEvent::Paint: {
      PresentTileMap();
      return true;
    }
    case QEvent::MouseButtonPress: {
      const QMouseEvent* mouse_event = (QMouseEvent*)event;

      if(mouse_event->button() == Qt::LeftButton) {
        const int canvas_tile_size = 8 * magnification_input->value();
        const int tile_x = (int)(mouse_event->x() / canvas_tile_size);
        const int tile_y = (int)(mouse_event->y() / canvas_tile_size);

        DrawTileDetail(tile_x, tile_y);
      } else {
        ClearTileSelection();
      }
      return true;
    }
  }

  return false;
}

void TileViewer::PresentTileMap() {
  const int canvas_w = canvas->size().width();
  const int canvas_h = canvas->size().height();

  const QRect src_rect{0, 0, 256, 256 * canvas_h / canvas_w};
  const QRect dst_rect{0, 0, canvas_w, canvas_h};

  QPainter painter{canvas};
  painter.drawImage(dst_rect, image_rgb32, src_rect);

  if(display_selected_tile) {
    const int tile_size = 8 * canvas_w / 256;

    painter.setPen(Qt::red);
    painter.drawRect(selected_tile_x * tile_size - 1, selected_tile_y * tile_size - 1, tile_size + 1, tile_size + 1);
  }
}

void TileViewer::DrawTileDetail(int tile_x, int tile_y) {
  if(!isEnabled()) {
    return;
  }

  tile_box->Draw(&image_rgb565[(tile_y * 256 + tile_x) * 8], 256);

  selected_tile_x = tile_x;
  selected_tile_y = tile_y;
  display_selected_tile = true;

  const int tile_number  = tile_y * 32 + tile_x;
  const u32 tile_address = 0x06000000u + tile_base + tile_number * (eight_bpp->isChecked() ? 64 : 32);

  label_tile_number->setText(QStringLiteral("%1").arg(tile_number));
  label_tile_address->setText(QString::fromStdString(fmt::format("0x{:08X}", tile_address)));

  canvas->update();
}

void TileViewer::ClearTileSelection() {
  if(!isEnabled()) {
    return;
  }

  display_selected_tile = false;
  canvas->update();
}

void TileViewer::Update() {
  if(isVisible()) {
    UpdateImpl();
  }
}

void TileViewer::UpdateImpl() {
  const int magnification = magnification_input->value();
  const int palette_offset = tile_base == 0x10000u ? 256 : 0;

  u32* const buffer = (u32*)image_rgb32.bits();

  int height = 256;
  u32 tile_address = tile_base;

  if(eight_bpp->isChecked()) {
    u16* const palette = &pram[palette_offset];

    for(int tile = 0; tile < 512; tile++) {
      const int tile_base_x = (tile % 32) * 8;
      const int tile_base_y = (tile / 32) * 8;

      for(int y = 0; y < 8; y++) {
        u64 tile_row_data = nba::read<u64>(vram, tile_address);

        for(int x = 0; x < 8; x++) {
          const size_t index = (tile_base_y + y) * 256 + tile_base_x + x;
          const u16 color_rgb565 = palette[(u8)tile_row_data];

          image_rgb565[index] = color_rgb565;
          buffer[index] = Rgb565ToArgb8888(color_rgb565);
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
          const size_t index = (tile_base_y + y) * 256 + tile_base_x + x;
          const u16 color_rgb565 = palette[tile_row_data & 15];

          image_rgb565[index] = color_rgb565;
          buffer[index] = Rgb565ToArgb8888(color_rgb565);
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