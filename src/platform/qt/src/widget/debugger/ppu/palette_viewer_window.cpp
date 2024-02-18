/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "palette_viewer_window.hpp"

PaletteViewerWindow::PaletteViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  const auto vbox = new QVBoxLayout{};

  m_palette_viewer = new PaletteViewer{core};
  vbox->addWidget(m_palette_viewer);
  vbox->setSizeConstraint(QLayout::SetFixedSize);

  setLayout(vbox);
  setWindowTitle(tr("Palette Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void PaletteViewerWindow::Update() {
  m_palette_viewer->Update();
}
