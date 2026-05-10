// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVBoxLayout>

#include "sprite_viewer_window.hh"

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
