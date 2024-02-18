/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QVBoxLayout>

#include "sprite_viewer_window.hpp"

SpriteViewerWindow::SpriteViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  QVBoxLayout* vbox = new QVBoxLayout{};

  m_sprite_viewer = new SpriteViewer{core, nullptr};
  vbox->addWidget(m_sprite_viewer);
  setLayout(vbox);

  setWindowTitle(tr("Sprite Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void SpriteViewerWindow::Update() {
  if(isVisible()) {
    m_sprite_viewer->Update();
  }
}