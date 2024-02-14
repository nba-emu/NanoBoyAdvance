/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QVBoxLayout>

#include "tile_viewer_window.hpp"

TileViewerWindow::TileViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  const auto vbox = new QVBoxLayout{};

  tile_viewer = new TileViewer{core, nullptr};
  vbox->addWidget(tile_viewer);
  setLayout(vbox);

  setWindowTitle(tr("Tile Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void TileViewerWindow::Update() {
  if(!isVisible()) {
    return;
  }
  tile_viewer->Update();
}