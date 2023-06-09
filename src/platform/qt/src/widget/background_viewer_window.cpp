/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <fmt/format.h>
#include <nba/common/punning.hpp>
#include <QEvent>
#include <QHBoxLayout>
#include <QScrollArea>

#include "background_viewer_window.hpp"

BackgroundViewerWindow::BackgroundViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent), core(core) {
  setWindowTitle(tr("Background Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  pram = (u16*)core->GetPRAM();
  vram = core->GetVRAM();

  setLayout(new QHBoxLayout{});

  test_label = new QLabel{"sus"};
  layout()->addWidget(test_label);

  canvas = new QWidget{};
  canvas->setFixedSize(1024, 1024); // TBD
  canvas->setStyleSheet("background-color: black;"); // TBD
  canvas->installEventFilter(this);

  const auto canvas_scroll_area = new QScrollArea{};
  canvas_scroll_area->setWidget(canvas);
  layout()->addWidget(canvas_scroll_area);
}

void BackgroundViewerWindow::Update() {
  if(!isVisible()) {
    return;
  }

  test_label->setText(QString::fromStdString(fmt::format("DISPCNT: 0x{:08X}", core->PeekHalfIO(0x04000000))));

  canvas->update();
}

bool BackgroundViewerWindow::eventFilter(QObject* object, QEvent* event) {
  if(object == canvas && event->type() == QEvent::Paint) {
    DrawBackground();
  }

  return false;
}

void BackgroundViewerWindow::DrawBackground() {
  // @todo: do all the actual drawing outside the event handler.
  
  QPainter painter{canvas};

  const u16 dispcnt = core->PeekHalfIO(0x04000000);
  const int mode = dispcnt & 7;

  switch(mode) {
    case 0: DrawBackgroundMode0(painter, 2); break;
    case 1: DrawBackgroundMode2(painter, 2); break;
    case 2: DrawBackgroundMode2(painter, 2); break;
    case 3: DrawBackgroundMode3(painter); break;
    case 4: DrawBackgroundMode4(painter); break;
    case 5: DrawBackgroundMode5(painter); break;
    default: {
      break;
    }
  }
}

void BackgroundViewerWindow::DrawBackgroundMode0(QPainter& painter, int bg_id) {
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

  QRect rect{0, 0, 1024, 1024};
  painter.drawImage(rect, image, rect);

  // Display visible area test
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
  }
}

void BackgroundViewerWindow::DrawBackgroundMode2(QPainter& painter, int bg_id) {
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

  QRect rect{0, 0, 1024, 1024};
  painter.drawImage(rect, image, rect);
}

void BackgroundViewerWindow::DrawBackgroundMode3(QPainter& painter) {
  u16* const buffer = (u16*)image.bits();

  u32 address = 0;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      buffer[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }

  QRect rect{0, 0, 1024, 1024};
  painter.drawImage(rect, image, rect);
}

void BackgroundViewerWindow::DrawBackgroundMode4(QPainter& painter) {
  u16* const buffer = (u16*)image.bits();

  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      buffer[y * 1024 + x] = pram[nba::read<u8>(vram, address++)];
    }
  }

  QRect rect{0, 0, 1024, 1024};
  painter.drawImage(rect, image, rect);
}

void BackgroundViewerWindow::DrawBackgroundMode5(QPainter& painter) {
  u16* const buffer = (u16*)image.bits();

  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 128; y++) {
    for(int x = 0; x < 160; x++) {
      buffer[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }

  QRect rect{0, 0, 1024, 1024};
  painter.drawImage(rect, image, rect);
}