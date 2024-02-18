/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fmt/format.h>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "widget/debugger/utility.hpp"
#include "palette_viewer.hpp"

PaletteViewer::PaletteViewer(nba::CoreBase* core, QWidget* parent) : QWidget(parent) {
  QVBoxLayout* vbox = new QVBoxLayout{};

  vbox->addWidget(new QLabel{tr("Select a color for detailed information:")});
  vbox->addLayout(CreatePaletteGrids());
  vbox->addLayout(CreateColorInfoArea());
  setLayout(vbox);

  m_pram = (u16*)core->GetPRAM();
}

QLayout* PaletteViewer::CreatePaletteGrids() {
  QHBoxLayout* hbox = new QHBoxLayout{};

  for(int grid = 0; grid < 2; grid++) {
    QVBoxLayout* vbox = new QVBoxLayout{};

    QGroupBox* group_box = new QGroupBox{};
    group_box->setTitle(grid == 0 ? tr("Background") : tr("Sprite"));
    group_box->setLayout(vbox);

    m_palette_color_grids[grid] = new ColorGrid{16, 16};

    connect(m_palette_color_grids[grid], &ColorGrid::selected, [=](int x, int y) {
      m_palette_color_grids[grid]->SetHighlightedPosition(x, y);
      m_palette_color_grids[grid ^ 1]->ClearHighlight();
      ShowColorInfo(grid << 8 | y << 4 | x);
    });

    vbox->addWidget(m_palette_color_grids[grid]);
    hbox->addWidget(group_box);
  }

  return hbox;
}

QLayout* PaletteViewer::CreateColorInfoArea() {
  QHBoxLayout* hbox = new QHBoxLayout{};

  m_label_color_address = CreateMonospaceLabel();
  m_label_color_r_component = CreateMonospaceLabel();
  m_label_color_g_component = CreateMonospaceLabel();
  m_label_color_b_component = CreateMonospaceLabel();
  m_label_color_value = CreateMonospaceLabel();

  QGridLayout* grid = new QGridLayout{};
  grid->addWidget(new QLabel{tr("Address:")}, 0, 0);
  grid->addWidget(m_label_color_address, 0, 1);
  grid->addWidget(new QLabel{tr("R:")}, 1, 0);
  grid->addWidget(m_label_color_r_component, 1, 1);
  grid->addWidget(new QLabel{tr("G:")}, 2, 0);
  grid->addWidget(m_label_color_g_component, 2, 1);
  grid->addWidget(new QLabel{tr("B:")}, 3, 0);
  grid->addWidget(m_label_color_b_component, 3, 1);
  grid->addWidget(new QLabel{tr("Value:")}, 4, 0);
  grid->addWidget(m_label_color_value, 4, 1);
  grid->setColumnStretch(1, 1);

  m_box_color_preview = new QWidget{};
  m_box_color_preview->setStyleSheet("background-color: black;");
  m_box_color_preview->setFixedSize(64, 64);

  hbox->addLayout(grid);
  hbox->addWidget(m_box_color_preview);
  return hbox;
}

void PaletteViewer::Update() {
  if(!isVisible()) {
    return;
  }

  m_palette_color_grids[0]->Draw(&m_pram[0], 16);
  m_palette_color_grids[1]->Draw(&m_pram[256], 16);
}

void PaletteViewer::ShowColorInfo(int color_index) {
  const u32 address = 0x05000000 + (color_index << 1);
  const int x = color_index & 15;
  const int y = (color_index >> 4) & 15;
  const u16 color = m_pram[color_index];

  const int r =  (color << 1) & 62;
  const int g = ((color >> 4) & 62) | (color >> 15);
  const int b =  (color >> 9) & 62;

  const u32 rgb = Rgb565ToArgb8888(color);

  m_label_color_address->setText(QString::fromStdString(fmt::format("0x{:08X} ({}, {})", address, x, y)));
  m_label_color_r_component->setText(QStringLiteral("%1").arg(r));
  m_label_color_g_component->setText(QStringLiteral("%1").arg(g));
  m_label_color_b_component->setText(QStringLiteral("%1").arg(b));
  m_label_color_value->setText(QString::fromStdString(fmt::format("0x{:04X}", color)));

  m_box_color_preview->setStyleSheet(QString::fromStdString(fmt::format("background-color: #{:06X};", rgb)));
}