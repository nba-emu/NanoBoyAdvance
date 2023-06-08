/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fmt/format.h>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "palette_viewer_window.hpp"

PaletteViewerWindow::PaletteViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  const auto vbox = new QVBoxLayout{};

  vbox->setSizeConstraint(QLayout::SetFixedSize);

  const auto palettes_hbox = new QHBoxLayout{};

  for(int box = 0; box < 2; box++) {
    const auto group_box = new QGroupBox{};
    const auto group_box_layout = new QVBoxLayout{};

    palette_boxes[box] = new PaletteBox{};

    connect(palette_boxes[box], &PaletteBox::selected, [=](int x, int y) {
      palette_boxes[box]->SetHighlightedPosition(x, y);
      palette_boxes[box ^ 1]->ClearHighlight();
      ShowColorInformation(box << 8 | y << 4 | x);
    });

    group_box->setTitle(box == 0 ? tr("Background") : tr("Sprite"));
    group_box_layout->addWidget(palette_boxes[box]);
    group_box->setLayout(group_box_layout);

    palettes_hbox->addWidget(group_box);
  }

  const auto monospace_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  address_label = new QLabel{};
  r_component_label = new QLabel{};
  g_component_label = new QLabel{};
  b_component_label = new QLabel{};
  value_label = new QLabel{};

  address_label->setFont(monospace_font);
  r_component_label->setFont(monospace_font);
  g_component_label->setFont(monospace_font);
  b_component_label->setFont(monospace_font);
  value_label->setFont(monospace_font);

  address_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  r_component_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  g_component_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  b_component_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  value_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

  const auto info_hbox = new QHBoxLayout{};

  const auto info_grid = new QGridLayout{};
  info_grid->addWidget(new QLabel{tr("Address:")}, 0, 0);
  info_grid->addWidget(address_label, 0, 1);
  info_grid->addWidget(new QLabel{tr("R:")}, 1, 0);
  info_grid->addWidget(r_component_label, 1, 1);
  info_grid->addWidget(new QLabel{tr("G:")}, 2, 0);
  info_grid->addWidget(g_component_label, 2, 1);
  info_grid->addWidget(new QLabel{tr("B:")}, 3, 0);
  info_grid->addWidget(b_component_label, 3, 1);
  info_grid->addWidget(new QLabel{tr("Value:")}, 4, 0);
  info_grid->addWidget(value_label, 4, 1);
  info_grid->setColumnStretch(1, 1);

  info_color = new QWidget{};
  info_color->setStyleSheet("background-color: black;");
  info_color->setFixedSize(64, 64);

  info_hbox->addLayout(info_grid);
  info_hbox->addWidget(info_color);

  vbox->addWidget(new QLabel{tr("Select a color for detailed information:")});
  vbox->addLayout(palettes_hbox);
  vbox->addLayout(info_hbox);

  setLayout(vbox);
  setWindowTitle(tr("Palette Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  pram = (u16*)core->GetPRAM();
}

void PaletteViewerWindow::Update() {
  if(!isVisible()) {
    return;
  }

  palette_boxes[0]->Draw(&pram[0]);
  palette_boxes[1]->Draw(&pram[256]);
}

void PaletteViewerWindow::ShowColorInformation(int color_index) {
  const u32 address = 0x05000000 + (color_index << 1);
  const int x = color_index & 15;
  const int y = (color_index >> 4) & 15;
  const u16 color = pram[color_index];

  const int r =  (color << 1) & 62;
  const int g = ((color >> 4) & 62) | (color >> 15);
  const int b =  (color >> 9) & 62;

  const u32 rgb = (r << 2 | r >> 3) << 16 |
                  (g << 2 | g >> 4) <<  8 |
                  (b << 2 | b >> 3);

  address_label->setText(QString::fromStdString(fmt::format("0x{:08X} ({}, {})", address, x, y)));
  r_component_label->setText(QStringLiteral("%1").arg(r));
  g_component_label->setText(QStringLiteral("%1").arg(g));
  b_component_label->setText(QStringLiteral("%1").arg(b));
  value_label->setText(QString::fromStdString(fmt::format("0x{:04X}", color)));

  info_color->setStyleSheet(QString::fromStdString(fmt::format("background-color: #{:06X};", rgb)));
}