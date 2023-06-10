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
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

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

  const auto info_vbox = new QVBoxLayout{};

  // Background information
  { 
    const auto grid = new QGridLayout{};

    int row = 0; 
    grid->addWidget(new QLabel(tr("BG mode:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_mode_label), row++, 1);
    grid->addWidget(new QLabel(tr("BG priority:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_priority_label), row++, 1);
    grid->addWidget(new QLabel(tr("Size:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_size_label), row++, 1);
    grid->addWidget(new QLabel(tr("Tile base:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_tile_base_label), row++, 1);
    grid->addWidget(new QLabel(tr("Map base:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_map_base_label), row++, 1);
    grid->addWidget(new QLabel(tr("8BPP:")), row, 0);
    grid->addWidget(CreateCheckBox(bg_8bpp_check_box), row++, 1);
    grid->addWidget(new QLabel(tr("Wraparound:")), row, 0);
    grid->addWidget(CreateCheckBox(bg_wraparound_check_box), row++, 1);
    grid->addWidget(new QLabel(tr("Scroll:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(bg_scroll_label), row++, 1);
    grid->setColumnStretch(1, 1);

    const auto group_box = new QGroupBox{};
    group_box->setLayout(grid);
    group_box->setTitle(tr("Background"));

    info_vbox->addWidget(group_box);
  }

  // Tile information
  {
    const auto grid = new QGridLayout{};

    tile_box = new PaletteBox{8, 8};

    int row = 0;
    grid->addWidget(new QLabel(tr("Tile number:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_number_label), row++, 1);
    grid->addWidget(new QLabel(tr("Tile address:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_address_label), row++, 1);
    grid->addWidget(new QLabel(tr("Map entry address:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_map_entry_address_label), row++, 1);
    grid->addWidget(new QLabel(tr("Flip V:")), row, 0);
    grid->addWidget(CreateCheckBox(tile_flip_v_check_box), row++, 1);
    grid->addWidget(new QLabel(tr("Flip H:")), row, 0);
    grid->addWidget(CreateCheckBox(tile_flip_h_check_box), row++, 1);
    grid->addWidget(new QLabel(tr("Palette:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_palette_label), row++, 1);
    grid->addWidget(tile_box, row++, 0);
    grid->addWidget(new QLabel(tr("R:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_color_r_component_label), row++, 1);
    grid->addWidget(new QLabel(tr("G:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_color_g_component_label), row++, 1);
    grid->addWidget(new QLabel(tr("B:")), row, 0);
    grid->addWidget(CreateMonospaceLabel(tile_color_b_component_label), row++, 1);

    const auto group_box = new QGroupBox{};
    group_box->setLayout(grid);
    group_box->setTitle(tr("Tile"));

    info_vbox->addWidget(group_box);
  
    connect(tile_box, &PaletteBox::selected, [this](int x, int y) {
      const u32 color_rgb32 = tile_box->GetColorAt(x, y);
      const int r = (color_rgb32 >> 18) & 62;
      const int g = (color_rgb32 >> 10) & 63;
      const int b = (color_rgb32 >>  2) & 62;

      tile_color_r_component_label->setText(QStringLiteral("%1").arg(r));
      tile_color_g_component_label->setText(QStringLiteral("%1").arg(g));
      tile_color_b_component_label->setText(QStringLiteral("%1").arg(b)); 

      tile_box->SetHighlightedPosition(x, y);
    });
  }

  display_screen_viewport_check_box = new QCheckBox{tr("Display screen viewport")};
  display_screen_viewport_check_box->setChecked(true);
  info_vbox->addWidget(display_screen_viewport_check_box);

  info_vbox->addStretch(1);
  ((QHBoxLayout*)layout())->addLayout(info_vbox);

  canvas = new QWidget{};
  canvas->setFixedSize(256, 256);
  canvas->installEventFilter(this);

  const auto canvas_scroll_area = new QScrollArea{};
  canvas_scroll_area->setWidget(canvas);
  layout()->addWidget(canvas_scroll_area);

  ((QHBoxLayout*)layout())->setStretch(1, 1);

  pram = (u16*)core->GetPRAM();
  vram = core->GetVRAM();

  image_rgb565 = new u16[1024 * 1024];
}

BackgroundViewer::~BackgroundViewer() {
  delete[] image_rgb565;
}

void BackgroundViewer::SetBackgroundID(int id) {
  bg_id = id;
}

void BackgroundViewer::Update() {
  if(!isVisible()) {
    return;
  }

  bg_mode = core->PeekHalfIO(0x04000000) & 7;

  switch(bg_mode) {
    case 0: setEnabled(true); break;
    case 1: setEnabled(bg_id != 3); break;
    case 2: setEnabled(bg_id >= 2); break;
    case 3:
    case 4:
    case 5: setEnabled(bg_id == 2); break;
    default: setEnabled(false);
  }

  if(!isEnabled()) {
    return;
  }

  const u16 bgcnt = core->PeekHalfIO(0x04000008 + (bg_id << 1));
  const int priority = bgcnt & 3;
  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  int width = 240;
  int height = 160;
  bool use_8bpp = false;
  bool wraparound = false;

  const bool text_mode = bg_mode == 0 || (bg_mode == 1 && bg_id < 2);

  if(text_mode) {
    width  = 256 << ((bgcnt >> 14) & 1);
    height = 256 <<  (bgcnt >> 15);
    use_8bpp = bgcnt & (1 << 7);
    bghofs = core->GetBGHOFS(bg_id);
    bgvofs = core->GetBGVOFS(bg_id);
  } else if(bg_mode <= 2) {
    width  = 128 << (bgcnt >> 14);
    height = width;
    use_8bpp = true;
    wraparound = bgcnt & (1 << 13);
  } else if(bg_mode == 5) {
    width  = 160;
    height = 128;
  }

  display_screen_viewport_check_box->setEnabled(text_mode);

  bg_mode_label->setText(QStringLiteral("%1").arg(bg_mode));
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

  switch(bg_mode) {
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

  canvas->setFixedSize(width, height);
  canvas->update();
}

bool BackgroundViewer::eventFilter(QObject* object, QEvent* event) {
  if(object != canvas) {
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
          const u32 map_entry_address = map_address + (y << 6 | x << 1);
          const u16 map_entry = nba::read<u16>(vram, map_entry_address);

          const int tile_number = map_entry & 0x3FF;
          const int flip_x = (map_entry & (1 << 10)) ? 7 : 0;
          const int flip_y = (map_entry & (1 << 11)) ? 7 : 0;
          const int palette = map_entry >> 12;

          auto& meta_data = tile_meta_data[screen_x << 5 | x][screen_y << 5 | y];

          meta_data.tile_number = tile_number;
          meta_data.map_entry_address = map_entry_address;
          meta_data.flip_v = flip_y > 0;
          meta_data.flip_h = flip_x > 0;

          if(use_8bpp) {
            u32 tile_address = tile_base + (tile_number << 6);

            meta_data.tile_address = tile_address;
            meta_data.palette = 0;

            for(int tile_y = 0; tile_y < 8; tile_y++) {
              u64 data = nba::read<u64>(vram, tile_address);

              const int image_y = screen_y << 8 | y << 3 | tile_y ^ flip_y;

              for(int tile_x = 0; tile_x < 8; tile_x++) {
                const int image_x = screen_x << 8 | x << 3 | tile_x ^ flip_x;

                image_rgb565[image_y * 1024 + image_x] = pram[(u8)data];
                data >>= 8;
              }

              tile_address += sizeof(u64);
            }
          } else {
            u32 tile_address = tile_base + (tile_number << 5);

            meta_data.tile_address = tile_address;
            meta_data.palette = palette;

            for(int tile_y = 0; tile_y < 8; tile_y++) {
              u32 data = nba::read<u32>(vram, tile_address);

              const int image_y = screen_y << 8 | y << 3 | tile_y ^ flip_y;

              for(int tile_x = 0; tile_x < 8; tile_x++) {
                const int image_x = screen_x << 8 | x << 3 | tile_x ^ flip_x;

                image_rgb565[image_y * 1024 + image_x] = pram[(palette << 4) | (data & 15)];
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
  const u16 bgcnt = core->PeekHalfIO(0x04000008 + (bg_id << 1));
  const int size = 128 << (bgcnt >> 14);
  
  const u32 tile_base = ((bgcnt >> 2) & 3) << 14;
  const u32 map_base = ((bgcnt >> 8) & 31) << 11;

  u32 map_address = map_base;

  // @todo: rename x and y - also in the mode 0 code

  for(int y = 0; y < size; y += 8) {
    for(int x = 0; x < size; x += 8) {
      const u8 tile_number = vram[map_address];

      u32 tile_address = tile_base + (tile_number << 6);

      auto& meta_data = tile_meta_data[x >> 3][y >> 3];

      meta_data.tile_number = tile_number;
      meta_data.tile_address = tile_address;
      meta_data.map_entry_address = map_address;
      meta_data.flip_v = false;
      meta_data.flip_h = false;
      meta_data.palette = 0;

      for(int tile_y = 0; tile_y < 8; tile_y++) {
        for(int tile_x = 0; tile_x < 8; tile_x++) {
          image_rgb565[(y + tile_y) * 1024 + x + tile_x] = pram[vram[tile_address++]];
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
      image_rgb565[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::DrawBackgroundMode4() {
  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      image_rgb565[y * 1024 + x] = pram[nba::read<u8>(vram, address++)];
    }
  }
}

void BackgroundViewer::DrawBackgroundMode5() {
  u32 address = (core->PeekHalfIO(0x04000000) & 0x10U) * 0xA00U;

  for(int y = 0; y < 128; y++) {
    for(int x = 0; x < 160; x++) {
      image_rgb565[y * 1024 + x] = nba::read<u16>(vram, address);
      address += sizeof(u16);
    }
  }
}

void BackgroundViewer::PresentBackground() {
  if(!isEnabled()) {
    return;
  }

  QPainter painter{canvas};

  const int width = canvas->size().width();
  const int height = canvas->size().height();

  u32* const destination = (u32*)image_rgb32.bits();

  const int skip = 1024 - width;

  int i = 0;

  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      const u16 color_rgb565 = image_rgb565[i];

      // @todo: de-duplicate the RGB565 to RGB32 conversion.
      const int r =  (color_rgb565 >>  0) & 31;
      const int g = ((color_rgb565 >>  4) & 62) | (color_rgb565 >> 15);
      const int b =  (color_rgb565 >> 10) & 31;

      destination[i++] = (r << 3 | r >> 2) << 16 | (g << 2 | g >> 4) <<  8 | (b << 3 | b >> 2);
    }

    i += skip;
  }

  QRect rect{0, 0, width, height};
  painter.drawImage(rect, image_rgb32, rect);

  if(display_selected_tile) {
    painter.setPen(Qt::red);
    painter.drawRect(selected_tile_x * 8 - 1, selected_tile_y * 8 - 1, 9, 9);
  }

  if(display_screen_viewport_check_box->isEnabled() && display_screen_viewport_check_box->isChecked()) {
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

void BackgroundViewer::DrawTileDetail(int tile_x, int tile_y) {
  if(!isEnabled()) {
    return;
  }

  tile_box->Draw(&image_rgb565[(tile_y * 1024 + tile_x) * 8], 1024);

  if(bg_mode <= 2) {
    auto& meta_data = tile_meta_data[tile_x][tile_y];

    tile_number_label->setText(QStringLiteral("%1").arg(meta_data.tile_number));
    tile_address_label->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + meta_data.tile_address)));
    tile_map_entry_address_label->setText(QString::fromStdString(fmt::format("0x{:08X}", 0x06000000 + meta_data.map_entry_address)));
    tile_flip_v_check_box->setChecked(meta_data.flip_v);
    tile_flip_h_check_box->setChecked(meta_data.flip_h);
    tile_palette_label->setText(QStringLiteral("%1").arg(meta_data.palette));
  } else {
    tile_number_label->setText("-");
    tile_address_label->setText("-");
    tile_map_entry_address_label->setText("-");
    tile_flip_v_check_box->setChecked(false);
    tile_flip_h_check_box->setChecked(false);
    tile_palette_label->setText("-");
  }

  selected_tile_x = tile_x;
  selected_tile_y = tile_y;
  display_selected_tile = true;

  canvas->update();
}

void BackgroundViewer::ClearTileSelection() {
  if(!isEnabled()) {
    return;
  }

  display_selected_tile = false;
  canvas->update();
}