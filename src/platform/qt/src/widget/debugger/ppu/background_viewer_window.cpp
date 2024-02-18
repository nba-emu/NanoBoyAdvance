/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QVBoxLayout>

#include "background_viewer_window.hpp"

BackgroundViewerWindow::BackgroundViewerWindow(nba::CoreBase* core, QWidget* parent) : QDialog(parent) {
  m_tab_widget = new QTabWidget{};

  for(int id = 0; id < 4; id++) {
    const auto bg_viewer = new BackgroundViewer{core};
    
    bg_viewer->SetBackgroundID(id);
    m_tab_widget->addTab(bg_viewer, QStringLiteral("BG%1").arg(id));
    m_bg_viewers[id] = bg_viewer;
  }

  setLayout(new QVBoxLayout{});
  layout()->addWidget(m_tab_widget);

  setWindowTitle(tr("Background Viewer"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void BackgroundViewerWindow::Update() {
  if(isVisible()) {
    m_bg_viewers[m_tab_widget->currentIndex()]->Update();
  }
}