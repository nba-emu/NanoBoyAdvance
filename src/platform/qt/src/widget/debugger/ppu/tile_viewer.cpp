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

  QGridLayout* grid = new QGridLayout{};
  grid->addWidget(new QLabel{tr("Magnification:")}, 0, 0);
  grid->addWidget(CreateMagnificationInput(), 0, 1);
  grid->addWidget(new QLabel{tr("Palette #:")}, 1, 0);
  grid->addWidget(CreatePaletteInput(), 1, 1);
  vbox_l->addLayout(grid);

  m_check_eight_bpp = new QCheckBox{tr("256 color mode (8BPP)")};
  vbox_l->addWidget(m_check_eight_bpp);

  vbox_l->addWidget(CreateTileBaseGroupBox());
  vbox_l->addWidget(CreateTileInfoGroupBox());
  vbox_l->addStretch();

  m_canvas = new QWidget{};
  m_canvas->installEventFilter(this);
  vbox_r->addWidget(m_canvas);
  vbox_r->addStretch();

  m_vram = core->GetVRAM();
  m_pram = (u16*)core->GetPRAM();
  m_image_rgb565 = new u16[256 * 256];

  UpdateImpl();
}

TileViewer::~TileViewer() {
  delete m_image_rgb565;
}

QWidget* TileViewer::CreateMagnificationInput() {
  m_spin_magnification = new QSpinBox{};
  m_spin_magnification->setMinimum(1);
  m_spin_magnification->setMaximum(16);
  connect(m_spin_magnification, QOverload<int>::of(&QSpinBox::valueChanged), [this](int _) { Update(); });
  return m_spin_magnification;
}

QWidget* TileViewer::CreatePaletteInput() {
  m_spin_palette_index = new QSpinBox{};
  m_spin_palette_index->setMinimum(0);
  m_spin_palette_index->setMaximum(15);
  return m_spin_magnification;
}

QWidget* TileViewer::CreateTileBaseGroupBox() {
  QVBoxLayout* vbox = new QVBoxLayout{};

  QGroupBox* group_box = new QGroupBox{};
  group_box->setTitle(tr("Tile Base"));
  group_box->setLayout(vbox);

  for(u32 m_tile_base = 0x06000000u; m_tile_base <= 0x06010000u; m_tile_base += 0x4000u) {
    QRadioButton* radio_button = new QRadioButton{
      QString::fromStdString(fmt::format("0x{:08X}", m_tile_base))};

    connect(radio_button, &QRadioButton::pressed, [this, m_tile_base]() {
      this->m_tile_base = m_tile_base & 0xFFFFFFu;
    });

    vbox->addWidget(radio_button);
    
    if(m_tile_base == 0x06000000u) radio_button->click();
  }

  return group_box;
}

QWidget* TileViewer::CreateTileInfoGroupBox() {
  QVBoxLayout* vbox = new QVBoxLayout{};

  QGroupBox* group_box = new QGroupBox{};
  group_box->setTitle(tr("Tile"));
  group_box->setLayout(vbox);

  QGridLayout* grid_1 = new QGridLayout{};
  m_label_tile_number  = CreateMonospaceLabel();
  m_label_tile_address = CreateMonospaceLabel();
  grid_1->addWidget(new QLabel{tr("Tile #:")}, 0, 0);
  grid_1->addWidget(m_label_tile_number, 0, 1);
  grid_1->addWidget(new QLabel{tr("Tile address:")}, 1, 0);
  grid_1->addWidget(m_label_tile_address, 1, 1);
  vbox->addLayout(grid_1);

  m_tile_box = new PaletteBox{8, 8};
  vbox->addWidget(m_tile_box);

  QGridLayout* grid_2 = new QGridLayout{};
  m_label_color_r_component = CreateMonospaceLabel();
  m_label_color_g_component = CreateMonospaceLabel();
  m_label_color_b_component = CreateMonospaceLabel();
  grid_2->addWidget(new QLabel{tr("R:")}, 0, 0);
  grid_2->addWidget(m_label_color_r_component, 0, 1);
  grid_2->addWidget(new QLabel{tr("G:")}, 1, 0);
  grid_2->addWidget(m_label_color_g_component, 1, 1);
  grid_2->addWidget(new QLabel{tr("B:")}, 2, 0);
  grid_2->addWidget(m_label_color_b_component, 2, 1);
  vbox->addLayout(grid_2);

  connect(m_tile_box, &PaletteBox::selected, [this](int x, int y) {
    const u32 color_rgb32 = m_tile_box->GetColorAt(x, y);
    const int r = (color_rgb32 >> 18) & 62;
    const int g = (color_rgb32 >> 10) & 63;
    const int b = (color_rgb32 >>  2) & 62;

    m_label_color_r_component->setText(QStringLiteral("%1").arg(r));
    m_label_color_g_component->setText(QStringLiteral("%1").arg(g));
    m_label_color_b_component->setText(QStringLiteral("%1").arg(b)); 

    m_tile_box->SetHighlightedPosition(x, y);
  });

  return group_box;
}

bool TileViewer::eventFilter(QObject* object, QEvent* event) {
  if(object != m_canvas) {
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
        const int m_canvas_tile_size = 8 * m_spin_magnification->value();
        const int tile_x = (int)(mouse_event->x() / m_canvas_tile_size);
        const int tile_y = (int)(mouse_event->y() / m_canvas_tile_size);

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
  const int m_canvas_w = m_canvas->size().width();
  const int m_canvas_h = m_canvas->size().height();

  const QRect src_rect{0, 0, 256, 256 * m_canvas_h / m_canvas_w};
  const QRect dst_rect{0, 0, m_canvas_w, m_canvas_h};

  QPainter painter{m_canvas};
  painter.drawImage(dst_rect, m_image_rgb32, src_rect);

  if(m_display_selected_tile) {
    const int tile_size = 8 * m_canvas_w / 256;

    painter.setPen(Qt::red);
    painter.drawRect(m_selected_tile_x * tile_size - 1, m_selected_tile_y * tile_size - 1, tile_size + 1, tile_size + 1);
  }
}

void TileViewer::DrawTileDetail(int tile_x, int tile_y) {
  if(!isEnabled()) {
    return;
  }

  m_tile_box->Draw(&m_image_rgb565[(tile_y * 256 + tile_x) * 8], 256);

  m_selected_tile_x = tile_x;
  m_selected_tile_y = tile_y;
  m_display_selected_tile = true;

  const int tile_number  = tile_y * 32 + tile_x;
  const u32 tile_address = 0x06000000u + m_tile_base + tile_number * (m_check_eight_bpp->isChecked() ? 64 : 32);

  m_label_tile_number->setText(QStringLiteral("%1").arg(tile_number));
  m_label_tile_address->setText(QString::fromStdString(fmt::format("0x{:08X}", tile_address)));

  m_canvas->update();
}

void TileViewer::ClearTileSelection() {
  if(!isEnabled()) {
    return;
  }

  m_display_selected_tile = false;
  m_canvas->update();
}

void TileViewer::Update() {
  if(isVisible()) {
    UpdateImpl();
  }
}

void TileViewer::UpdateImpl() {
  const int magnification = m_spin_magnification->value();
  const int palette_offset = m_tile_base == 0x10000u ? 256 : 0;

  u16* const image_rgb565 = m_image_rgb565; 
  u32* const image_rgb32  = (u32*)m_image_rgb32.bits();

  int height = 256;
  u32 tile_address = m_tile_base;

  if(m_check_eight_bpp->isChecked()) {
    u16* const palette = &m_pram[palette_offset];

    for(int tile = 0; tile < 512; tile++) {
      const int m_tile_base_x = (tile % 32) * 8;
      const int m_tile_base_y = (tile / 32) * 8;

      for(int y = 0; y < 8; y++) {
        u64 tile_row_data = nba::read<u64>(m_vram, tile_address);

        for(int x = 0; x < 8; x++) {
          const size_t index = (m_tile_base_y + y) * 256 + m_tile_base_x + x;
          const u16 color_rgb565 = palette[(u8)tile_row_data];

          image_rgb565[index] = color_rgb565;
          image_rgb32[index] = Rgb565ToArgb8888(color_rgb565);
          tile_row_data >>= 8;
        }

        tile_address += sizeof(u64);
      }
    }

    height /= 2;
  } else {
    u16* const palette = &m_pram[m_spin_palette_index->value() * 16 + palette_offset];

    for(int tile = 0; tile < 1024; tile++) {
      const int m_tile_base_x = (tile % 32) * 8;
      const int m_tile_base_y = (tile / 32) * 8;

      for(int y = 0; y < 8; y++) {
        u32 tile_row_data = nba::read<u32>(m_vram, tile_address);

        for(int x = 0; x < 8; x++) {
          const size_t index = (m_tile_base_y + y) * 256 + m_tile_base_x + x;
          const u16 color_rgb565 = palette[tile_row_data & 15];

          image_rgb565[index] = color_rgb565;
          image_rgb32[index] = Rgb565ToArgb8888(color_rgb565);
          tile_row_data >>= 4;
        }

        tile_address += sizeof(u32);
      }
    }
  }

  if(m_tile_base == 0xC000u) {
    height /= 2;
  }

  m_canvas->setFixedSize(256 * magnification, height * magnification);
  m_canvas->update();
}