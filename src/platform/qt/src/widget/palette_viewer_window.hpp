/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QLabel>

#include "widget/palette_box.hpp"

struct PaletteViewerWindow : QDialog {
  PaletteViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

public slots:
  void Update();

private:
  void ShowColorInformation(int color_index);

  u16* pram;
  PaletteBox* palette_boxes[2];
  QLabel* address_label;
  QLabel* r_component_label;
  QLabel* g_component_label;
  QLabel* b_component_label;
  QLabel* value_label;
  QWidget* info_color;

  Q_OBJECT
};
