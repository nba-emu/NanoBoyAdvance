/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <fmt/format.h>
#include <nba/common/punning.hpp>
#include <optional>
#include <QEvent>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>

#include "background_viewer.hpp"

BackgroundViewer::BackgroundViewer(nba::CoreBase* core, QWidget* parent) : QWidget(parent), core(core) {
  setLayout(new QHBoxLayout{});

  const auto monospace_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  const auto CreateMonospaceLabel = [&](QLabel*& label) {
    label = new QLabel{"-"};
    label->setFont(monospace_font);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
  };

  const auto CreateCheckBox = [&](QCheckBox*& check_box) {
    check_box = new QCheckBox{};
    check_box->setEnabled(false);
    return check_box;
  };

  const auto info_grid = new QGridLayout{};
  int row = 0;
  info_grid->addWidget(new QLabel(tr("BG mode:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_mode_label), row++, 1);
  info_grid->addWidget(new QLabel(tr("BG priority:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_priority_label), row++, 1);
  info_grid->addWidget(new QLabel(tr("Size:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_size_label), row++, 1);
  info_grid->addWidget(new QLabel(tr("Tile base:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_tile_base_label), row++, 1);
  info_grid->addWidget(new QLabel(tr("Map base:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_map_base_label), row++, 1);
  info_grid->addWidget(new QLabel(tr("8BPP:")), row, 0);
  info_grid->addWidget(CreateCheckBox(bg_8bpp_check_box), row++, 1);
  info_grid->addWidget(new QLabel(tr("Wraparound:")), row, 0);
  info_grid->addWidget(CreateCheckBox(bg_wraparound_check_box), row++, 1);
  info_grid->addWidget(new QLabel(tr("Scroll:")), row, 0);
  info_grid->addWidget(CreateMonospaceLabel(bg_scroll_label), row++, 1);
  info_grid->setColumnStretch(1, 1);

  const auto info_group_box = new QGroupBox{};
  info_group_box->setLayout(info_grid);
  info_group_box->setTitle("Background");
  layout()->addWidget(info_group_box);

  canvas = new QWidget{};
  canvas->setFixedSize(256, 256);
  canvas->installEventFilter(this);

  const auto canvas_scroll_area = new QScrollArea{};
  canvas_scroll_area->setWidget(canvas);
  layout()->addWidget(canvas_scroll_area);

  ((QHBoxLayout*)layout())->setStretch(1, 1);

  pram = (u16*)core->GetPRAM();
  vram = core->GetVRAM();
}

void BackgroundViewer::SetBackgroundID(int id) {
  bg_id = id;
}

void BackgroundViewer::Update() {
  if(!isVisible()) {
    return;
  }

  // @todo: handle case when background is not available in the current mode.

  const u16 dispcnt = core->PeekHalfIO(0x04000000);
  const u16 bgcnt = core->PeekHalfIO(0x04000008 + (bg_id << 1));
  const u16 bghofs = 32; // @todo
  const u16 bgvofs = 16; // @todo

  const int mode = dispcnt & 7;
  const int priority = bgcnt & 3;

  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  int width = 240;
  int height = 160;
  bool use_8bpp = false;
  bool wraparound = false;

  const bool text_mode = mode == 0 || (mode == 1 && bg_id < 2);

  if(text_mode) {
    width  = 256 << ((bgcnt >> 14) & 1);
    height = 256 <<  (bgcnt >> 15);
    use_8bpp = bgcnt & (1 << 7);
  } else if(mode <= 2) {
    width  = 128 << (bgcnt >> 14);
    height = width;
    use_8bpp = true;
    wraparound = bgcnt & (1 << 13);
  } else if(mode == 5) {
    width  = 160;
    height = 128;
  }

  bg_mode_label->setText(QStringLiteral("%1").arg(mode));
  bg_priority_label->setText(QStringLiteral("%1").arg(priority));
  bg_size_label->setText(QString::fromStdString(fmt::format("{} x {}", width, height)));
  bg_tile_base_label->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + tile_base)));
  bg_map_base_label->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + map_base)));
  bg_8bpp_check_box->setChecked(use_8bpp);
  bg_wraparound_check_box->setChecked(wraparound);
  if(text_mode) {
    bg_scroll_label->setText(QString::fromStdString(fmt::format("{}, {}", bghofs, bgvofs)));
  } else {
    bg_scroll_label->setText("-");
  }

  canvas->setFixedSize(width, height);

  DrawBackground();

  canvas->update();
}

bool BackgroundViewer::eventFilter(QObject* object, QEvent* event) {
  if(object == canvas && event->type() == QEvent::Paint) {
    PresentBackground();
  }

  return false;
}

void BackgroundViewer::DrawBackground() {
  const u16 dispcnt = core->PeekHalfIO(0x04000000);
  const int mode = dispcnt & 7;

  switch(mode) {
    case 0: DrawBackgroundMode0(); break;
    case 1: {
      if(bg_id < 2) {
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
}

void BackgroundViewer::DrawBackgroundMode0() {
  u16* const buffer = (u16*)image.bits();
  
  const u16 bgcnt = core->PeekHalfIO(0x04000008 + (bg_id << 1));

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
          const u16 map_entry = nba::read<u16>(vram, map_address + (y << 6 | x << 1));

          const int tile_number = map_entry & 0x3FF;
          const int flip_x = (map_entry & (1 << 10)) ? 7 : 0;
          const int flip_y = (map_entry & (1 << 11)) ? 7 : 0;
          const int palette = map_entry >> 12;

          if(use_8bpp) {
            // @todo
          } else {
            u32 tile_address = tile_base + (tile_number << 5);

            for(int tile_y = 0; tile_y < 8; tile_y++) {
              u32 data = nba::read<u32>(vram, tile_address);

              const int image_y = screen_y << 8 | y << 3 | tile_y ^ flip_y;

              for(int tile_x = 0; tile_x < 8; tile_x++) {
                const int image_x = screen_x << 8 | x << 3 | tile_x ^ flip_x;

                buffer[image_y * 1024 + image_x] = pram[(palette << 4) | (data & 15)];
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
  u16* const buffer = (u16*)image.bits();

  const u16 bgcnt = core->PeekHalfIO(0x04000008 + (bg_id << 1));
  const int size = 128 << (bgcnt >> 14);
  
  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  u32 map_address = map_base;

  // @todo: rename x and y - also in the mode 0 code

  for(int y = 0; y < size; y += 8) {
    for(int x = 0; x < size; x += 8) {
      const u8 tile = vram[map_address++];

      u32 tile_address = tile_base + (tile << 6);

      for(int tile_y = 0; tile_y < 8; tile_y++) {
        for(int tile_x = 0; tile_x < 8; tile_x++) {
          buffer[(y + tile_y) * 1024 + x + tile_x] = pram[vram[tile_address++]];
        }
      }
    }
  }
}

void BackgroundViewer::DrawBackgroundMode3() {
  u16* const buffer = (u16*)image.bits();

  u32 address = 0;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      buffer[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::DrawBackgroundMode4() {
  u16* const buffer = (u16*)image.bits();

  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      buffer[y * 1024 + x] = pram[nba::read<u8>(vram, address++)];
    }
  }
}

void BackgroundViewer::DrawBackgroundMode5() {
  u16* const buffer = (u16*)image.bits();

  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 128; y++) {
    for(int x = 0; x < 160; x++) {
      buffer[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::PresentBackground() {
  QPainter painter{canvas};

  QRect rect{0, 0, canvas->size().width(), canvas->size().height()};
  painter.drawImage(rect, image, rect);

  /*// Display visible area test
  {
    const u16 bghofs = 340;//core->PeekHalfIO(0x04000010 + (bg_id << 2));
    const u16 bgvofs = 240;//core->PeekHalfIO(0x04000012 + (bg_id << 2));

    const int width  = 256 * screens_x;
    const int height = 256 * screens_y;

    painter.setPen(Qt::red);

    const int x_min = bghofs % width;
    const int x_max = (x_min + 239) % width;

    const int y_min = bgvofs % height;
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
  }*/
}
