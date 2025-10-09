/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <fmt/format.h>
#include <nba/common/punning.hpp>
#include <optional>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

#include "widget/debugger/utility.hpp"
#include "background_viewer.hpp"

BackgroundViewer::BackgroundViewer(nba::CoreBase* core, QWidget* parent) : QWidget(parent), m_core(core) {
  QHBoxLayout* hbox = new QHBoxLayout{};

  setLayout(hbox);
  
  hbox->addLayout(CreateInfoPanel());
  hbox->addWidget(CreateCanvasScrollArea());
  hbox->setStretch(1, 1);

  m_pram = (u16*)core->GetPRAM();
  m_vram = core->GetVRAM();

  m_image_rgb565 = new u16[1024 * 1024];
}

BackgroundViewer::~BackgroundViewer() {
  delete[] m_image_rgb565;
}

QWidget* BackgroundViewer::CreateBackgroundInfoGroupBox() {
  QGridLayout* grid = new QGridLayout{};

  QGroupBox* group_box = new QGroupBox{};
  group_box->setTitle(tr("Background"));
  group_box->setLayout(grid);

  m_label_bg_mode = CreateMonospaceLabel();
  m_label_bg_priority = CreateMonospaceLabel();
  m_label_bg_size = CreateMonospaceLabel();
  m_label_tile_base = CreateMonospaceLabel();
  m_label_map_base = CreateMonospaceLabel();
  m_check_8bpp = CreateReadOnlyCheckBox();
  m_check_bg_wraparound = CreateReadOnlyCheckBox();
  m_label_bg_scroll = CreateMonospaceLabel();

  int row = 0; 
  grid->addWidget(new QLabel(tr("BG mode:")), row, 0);
  grid->addWidget(m_label_bg_mode, row++, 1);
  grid->addWidget(new QLabel(tr("BG priority:")), row, 0);
  grid->addWidget(m_label_bg_priority, row++, 1);
  grid->addWidget(new QLabel(tr("Size:")), row, 0);
  grid->addWidget(m_label_bg_size, row++, 1);
  grid->addWidget(new QLabel(tr("Tile base:")), row, 0);
  grid->addWidget(m_label_tile_base, row++, 1);
  grid->addWidget(new QLabel(tr("Map base:")), row, 0);
  grid->addWidget(m_label_map_base, row++, 1);
  grid->addWidget(new QLabel(tr("8BPP:")), row, 0);
  grid->addWidget(m_check_8bpp, row++, 1);
  grid->addWidget(new QLabel(tr("Wraparound:")), row, 0);
  grid->addWidget(m_check_bg_wraparound, row++, 1);
  grid->addWidget(new QLabel(tr("Scroll:")), row, 0);
  grid->addWidget(m_label_bg_scroll, row++, 1);
  grid->setColumnStretch(1, 1);

  return group_box;
}

QWidget* BackgroundViewer::CreateTileInfoGroupBox() {
  QGridLayout* grid = new QGridLayout{};

  QGroupBox* group_box = new QGroupBox{};
  group_box->setTitle(tr("Tile"));
  group_box->setLayout(grid);

  m_tile_color_grid = new ColorGrid{8, 8};

  m_label_tile_number = CreateMonospaceLabel();
  m_label_tile_address = CreateMonospaceLabel();
  m_label_tile_map_entry_address = CreateMonospaceLabel();
  m_check_tile_flip_v = CreateReadOnlyCheckBox();
  m_check_tile_flip_h = CreateReadOnlyCheckBox();
  m_label_tile_palette = CreateMonospaceLabel();
  m_label_color_r_component = CreateMonospaceLabel();
  m_label_color_g_component = CreateMonospaceLabel();
  m_label_color_b_component = CreateMonospaceLabel();

  int row = 0;
  grid->addWidget(new QLabel(tr("Tile #:")), row, 0);
  grid->addWidget(m_label_tile_number, row++, 1);
  grid->addWidget(new QLabel(tr("Tile address:")), row, 0);
  grid->addWidget(m_label_tile_address, row++, 1);
  grid->addWidget(new QLabel(tr("Map entry address:")), row, 0);
  grid->addWidget(m_label_tile_map_entry_address, row++, 1);
  grid->addWidget(new QLabel(tr("Flip V:")), row, 0);
  grid->addWidget(m_check_tile_flip_v, row++, 1);
  grid->addWidget(new QLabel(tr("Flip H:")), row, 0);
  grid->addWidget(m_check_tile_flip_h, row++, 1);
  grid->addWidget(new QLabel(tr("Palette:")), row, 0);
  grid->addWidget(m_label_tile_palette, row++, 1);
  grid->addWidget(m_tile_color_grid, row++, 0);
  grid->addWidget(new QLabel(tr("R:")), row, 0);
  grid->addWidget(m_label_color_r_component, row++, 1);
  grid->addWidget(new QLabel(tr("G:")), row, 0);
  grid->addWidget(m_label_color_g_component, row++, 1);
  grid->addWidget(new QLabel(tr("B:")), row, 0);
  grid->addWidget(m_label_color_b_component, row++, 1);

  connect(m_tile_color_grid, &ColorGrid::selected, [this](int x, int y) {
    const u32 color_rgb32 = m_tile_color_grid->GetColorAt(x, y);
    const int r = (color_rgb32 >> 18) & 62;
    const int g = (color_rgb32 >> 10) & 63;
    const int b = (color_rgb32 >>  2) & 62;

    m_label_color_r_component->setText(QStringLiteral("%1").arg(r));
    m_label_color_g_component->setText(QStringLiteral("%1").arg(g));
    m_label_color_b_component->setText(QStringLiteral("%1").arg(b)); 

    m_tile_color_grid->SetHighlightedPosition(x, y);
  });

  return group_box;
}

QLayout* BackgroundViewer::CreateInfoPanel() {
  QVBoxLayout* vbox = new QVBoxLayout{};

  m_check_display_screen_viewport = new QCheckBox{tr("Display screen viewport")};
  m_check_display_screen_viewport->setChecked(true);
  
  vbox->addWidget(CreateBackgroundInfoGroupBox());
  vbox->addWidget(CreateTileInfoGroupBox());
  vbox->addWidget(m_check_display_screen_viewport);
  vbox->addStretch(1);
  return vbox;
}

QWidget* BackgroundViewer::CreateCanvasScrollArea() {
  QScrollArea* canvas_scroll_area = new QScrollArea{};
  m_canvas = new QWidget{};
  m_canvas->setFixedSize(256, 256);
  m_canvas->installEventFilter(this);
  canvas_scroll_area->setWidget(m_canvas);
  return canvas_scroll_area;
}

void BackgroundViewer::SetBackgroundID(int id) {
  m_bg_id = id;
}

void BackgroundViewer::Update() {
  if(!isVisible()) {
    return;
  }

  m_bg_mode = m_core->PeekHalfIO(0x04000000) & 7;

  switch(m_bg_mode) {
    case 0: setEnabled(true); break;
    case 1: setEnabled(m_bg_id != 3); break;
    case 2: setEnabled(m_bg_id >= 2); break;
    case 3:
    case 4:
    case 5: setEnabled(m_bg_id == 2); break;
    default: setEnabled(false);
  }

  if(!isEnabled()) {
    return;
  }

  const u16 bgcnt = m_core->PeekHalfIO(0x04000008 + (m_bg_id << 1));
  const int priority = bgcnt & 3;
  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  int width = 240;
  int height = 160;
  bool use_8bpp = false;
  bool wraparound = false;

  const bool text_mode = m_bg_mode == 0 || (m_bg_mode == 1 && m_bg_id < 2);

  if(text_mode) {
    width  = 256 << ((bgcnt >> 14) & 1);
    height = 256 <<  (bgcnt >> 15);
    use_8bpp = bgcnt & (1 << 7);
    m_bghofs = m_core->GetBGHOFS(m_bg_id);
    m_bgvofs = m_core->GetBGVOFS(m_bg_id);
  } else if(m_bg_mode <= 2) {
    width  = 128 << (bgcnt >> 14);
    height = width;
    use_8bpp = true;
    wraparound = bgcnt & (1 << 13);
  } else if(m_bg_mode == 5) {
    width  = 160;
    height = 128;
  }

  m_check_display_screen_viewport->setEnabled(text_mode);

  m_label_bg_mode->setText(QStringLiteral("%1").arg(m_bg_mode));
  m_label_bg_priority->setText(QStringLiteral("%1").arg(priority));
  m_label_bg_size->setText(QString::fromStdString(fmt::format("{} x {}", width, height)));
  m_label_tile_base->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + tile_base)));
  m_label_map_base->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + map_base)));
  m_check_8bpp->setChecked(use_8bpp);
  m_check_bg_wraparound->setChecked(wraparound);
  if(text_mode) {
    m_label_bg_scroll->setText(QString::fromStdString(fmt::format("{}, {}", m_bghofs, m_bgvofs)));
  } else {
    m_label_bg_scroll->setText("-");
  }

  switch(m_bg_mode) {
    case 0: DrawBackgroundMode0(); break;
    case 1: {
      if(m_bg_id < 2) {
        DrawBackgroundMode0();
      } else {
        DrawBackgroundMode2();
      }
      break;
    }
    case 2: DrawBackgroundMode2(); break;
    case 3: DrawBackgroundMode3(); break;
    case 4: DrawBackgroundMode4(); break;
    case 5: DrawBackgroundMode5(); break;
    default: {
      break;
    }
  }

  m_canvas->setFixedSize(width, height);
  m_canvas->update();
}

bool BackgroundViewer::eventFilter(QObject* object, QEvent* event) {
  if(object != m_canvas) {
    return false;
  }

  switch(event->type()) {
    case QEvent::Paint: {
      PresentBackground();
      break;
    }
    case QEvent::MouseButtonPress: {
      const QMouseEvent* mouse_event = (QMouseEvent*)event;

      if(mouse_event->button() == Qt::LeftButton) {
        const int tile_x = (int)(mouse_event->x() / 8);
        const int tile_y = (int)(mouse_event->y() / 8);

        DrawTileDetail(tile_x, tile_y);
      } else {
        ClearTileSelection();
      }
      break;
    }
  }

  return false;
}

void BackgroundViewer::DrawBackgroundMode0() {
  const u16 bgcnt = m_core->PeekHalfIO(0x04000008 + (m_bg_id << 1));

  const int screens_x = 1 + ((bgcnt >> 14) & 1);
  const int screens_y = 1 + (bgcnt >> 15);

  const bool use_8bpp = bgcnt & (1 << 7);

  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  u32 map_address = map_base;

  for(int screen_y = 0; screen_y < screens_y; screen_y++) {
    for(int screen_x = 0; screen_x < screens_x; screen_x++) {
      for(int y = 0; y < 32; y++) {
        for(int x = 0; x < 32; x++) {
          const u32 map_entry_address = map_address + (y << 6 | x << 1);
          const u16 map_entry = nba::read<u16>(m_vram, map_entry_address);

          const int tile_number = map_entry & 0x3FF;
          const int flip_x = (map_entry & (1 << 10)) ? 7 : 0;
          const int flip_y = (map_entry & (1 << 11)) ? 7 : 0;
          const int palette = map_entry >> 12;

          auto& meta_data = m_tile_meta_data[screen_x << 5 | x][screen_y << 5 | y];

          meta_data.tile_number = tile_number;
          meta_data.map_entry_address = map_entry_address;
          meta_data.flip_v = flip_y > 0;
          meta_data.flip_h = flip_x > 0;

          if(use_8bpp) {
            u32 tile_address = tile_base + (tile_number << 6);

            meta_data.tile_address = tile_address;
            meta_data.palette = 0;

            for(int tile_y = 0; tile_y < 8; tile_y++) {
              u64 data = nba::read<u64>(m_vram, tile_address);

              const int image_y = screen_y << 8 | y << 3 | tile_y ^ flip_y;

              for(int tile_x = 0; tile_x < 8; tile_x++) {
                const int image_x = screen_x << 8 | x << 3 | tile_x ^ flip_x;

                m_image_rgb565[image_y * 1024 + image_x] = m_pram[(u8)data];
                data >>= 8;
              }

              tile_address += sizeof(u64);
            }
          } else {
            u32 tile_address = tile_base + (tile_number << 5);

            meta_data.tile_address = tile_address;
            meta_data.palette = palette;

            for(int tile_y = 0; tile_y < 8; tile_y++) {
              u32 data = nba::read<u32>(m_vram, tile_address);

              const int image_y = screen_y << 8 | y << 3 | tile_y ^ flip_y;

              for(int tile_x = 0; tile_x < 8; tile_x++) {
                const int image_x = screen_x << 8 | x << 3 | tile_x ^ flip_x;

                m_image_rgb565[image_y * 1024 + image_x] = m_pram[(palette << 4) | (data & 15)];
                data >>= 4;
              }

              tile_address += sizeof(u32);
            }
          }
        }
      }

      map_address += 2048;
    }
  }
}

void BackgroundViewer::DrawBackgroundMode2() {
  const u16 bgcnt = m_core->PeekHalfIO(0x04000008 + (m_bg_id << 1));
  const int size = 128 << (bgcnt >> 14);
  
  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  u32 map_address = map_base;

  // @todo: rename x and y - also in the mode 0 code

  for(int y = 0; y < size; y += 8) {
    for(int x = 0; x < size; x += 8) {
      const u8 tile_number = m_vram[map_address];

      u32 tile_address = tile_base + (tile_number << 6);

      auto& meta_data = m_tile_meta_data[x >> 3][y >> 3];

      meta_data.tile_number = tile_number;
      meta_data.tile_address = tile_address;
      meta_data.map_entry_address = map_address;
      meta_data.flip_v = false;
      meta_data.flip_h = false;
      meta_data.palette = 0;

      for(int tile_y = 0; tile_y < 8; tile_y++) {
        for(int tile_x = 0; tile_x < 8; tile_x++) {
          m_image_rgb565[(y + tile_y) * 1024 + x + tile_x] = m_pram[m_vram[tile_address++]];
        }
      }

      map_address++;
    }
  }
}

void BackgroundViewer::DrawBackgroundMode3() {
  u32 address = 0;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      m_image_rgb565[y * 1024 + x] = nba::read<u16>(m_vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::DrawBackgroundMode4() {
  u32 address = (m_core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      m_image_rgb565[y * 1024 + x] = m_pram[nba::read<u8>(m_vram, address++)];
    }
  }
}

void BackgroundViewer::DrawBackgroundMode5() {
  u32 address = (m_core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 128; y++) {
    for(int x = 0; x < 160; x++) {
      m_image_rgb565[y * 1024 + x] = nba::read<u16>(m_vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::PresentBackground() {
  if(!isEnabled()) {
    return;
  }

  QPainter painter{m_canvas};

  const int width = m_canvas->size().width();
  const int height = m_canvas->size().height();

  u32* const destination = (u32*)m_image_rgb32.bits();

  const int skip = 1024 - width;

  int i = 0;

  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      destination[i++] = Rgb565ToArgb8888(m_image_rgb565[i]);
    }

    i += skip;
  }

  QRect rect{0, 0, width, height};
  painter.drawImage(rect, m_image_rgb32, rect);

  if(m_display_selected_tile) {
    painter.setPen(Qt::red);
    painter.drawRect(m_selected_tile_x * 8 - 1, m_selected_tile_y * 8 - 1, 9, 9);
  }

  if(m_check_display_screen_viewport->isEnabled() && m_check_display_screen_viewport->isChecked()) {
    painter.setPen(Qt::red);

    const int x_min = m_bghofs % width;
    const int x_max = (x_min + 239) % width;

    const int y_min = m_bgvofs % height;
    const int y_max = (y_min + 159) % height;

    if(x_max < x_min) {
      painter.drawLine(x_min, y_min, width - 1, y_min);
      painter.drawLine(x_min, y_max, width - 1, y_max);
      
      painter.drawLine(0, y_min, x_max, y_min);
      painter.drawLine(0, y_max, x_max, y_max);
    } else {
      painter.drawLine(x_min, y_min, x_max, y_min);
      painter.drawLine(x_min, y_max, x_max, y_max);
    }

    if(y_max < y_min) {
      painter.drawLine(x_min, y_min, x_min, height - 1);
      painter.drawLine(x_max, y_min, x_max, height - 1);

      painter.drawLine(x_min, 0, x_min, y_max);
      painter.drawLine(x_max, 0, x_max, y_max);
    } else {
      painter.drawLine(x_min, y_min, x_min, y_max);
      painter.drawLine(x_max, y_min, x_max, y_max);
    }
  }
}

void BackgroundViewer::DrawTileDetail(int tile_x, int tile_y) {
  if(!isEnabled()) {
    return;
  }

  m_tile_color_grid->Draw(&m_image_rgb565[(tile_y * 1024 + tile_x) * 8], 1024);

  if(m_bg_mode <= 2) {
    auto& meta_data = m_tile_meta_data[tile_x][tile_y];

    m_label_tile_number->setText(QStringLiteral("%1").arg(meta_data.tile_number));
    m_label_tile_address->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + meta_data.tile_address)));
    m_label_tile_map_entry_address->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + meta_data.map_entry_address)));
    m_check_tile_flip_v->setChecked(meta_data.flip_v);
    m_check_tile_flip_h->setChecked(meta_data.flip_h);
    m_label_tile_palette->setText(QStringLiteral("%1").arg(meta_data.palette));
  } else {
    m_label_tile_number->setText("-");
    m_label_tile_address->setText("-");
    m_label_tile_map_entry_address->setText("-");
    m_check_tile_flip_v->setChecked(false);
    m_check_tile_flip_h->setChecked(false);
    m_label_tile_palette->setText("-");
  }

  m_selected_tile_x = tile_x;
  m_selected_tile_y = tile_y;
  m_display_selected_tile = true;

  m_canvas->update();
}

void BackgroundViewer::ClearTileSelection() {
  if(isEnabled()) {
    m_display_selected_tile = false;
    m_canvas->update();
  }
}