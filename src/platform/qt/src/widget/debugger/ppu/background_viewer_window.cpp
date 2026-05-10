// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

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
