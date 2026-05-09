/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QVBoxLayout>

#include "tile_viewer_window.hpp"

TileViewerWindow::TileViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  QVBoxLayout* vbox = new QVBoxLayout{};

  m_tile_viewer = new TileViewer{core, nullptr};
  vbox->addWidget(m_tile_viewer);
  setLayout(vbox);

  setWindowTitle(tr("Tile Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void TileViewerWindow::Update() {
  if(isVisible()) {
    m_tile_viewer->Update();
  }
}
