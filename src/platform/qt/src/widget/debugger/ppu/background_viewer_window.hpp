// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QTabWidget>

#include "background_viewer.hpp"

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
