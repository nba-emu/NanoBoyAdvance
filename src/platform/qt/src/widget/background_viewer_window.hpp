/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QTabWidget>

#include "widget/background_viewer.hpp"

struct BackgroundViewerWindow : QDialog {
  BackgroundViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

public slots:
  void Update();

private:
  QTabWidget* tab_widget;
  BackgroundViewer* bg_viewers[4];

  Q_OBJECT
};
