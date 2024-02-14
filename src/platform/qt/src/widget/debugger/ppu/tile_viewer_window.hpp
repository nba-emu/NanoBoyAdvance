/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QLabel>

#include "tile_viewer.hpp"

struct TileViewerWindow : QDialog {
  TileViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

public slots:
  void Update();

private:
  TileViewer* tile_viewer;

  Q_OBJECT
};
