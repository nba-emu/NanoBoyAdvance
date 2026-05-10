// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <QDialog>
#include <QTabWidget>

#include "background_viewer.hh"

class BackgroundViewerWindow : public QDialog {
  public:
    BackgroundViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  public slots:
    void Update();

  private:
    QTabWidget* m_tab_widget;
    BackgroundViewer* m_bg_viewers[4];

    Q_OBJECT
};
