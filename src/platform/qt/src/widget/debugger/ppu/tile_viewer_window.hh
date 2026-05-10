// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <QDialog>
#include <QLabel>

#include "tile_viewer.hh"

class TileViewerWindow : public QDialog {
  public:
    TileViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  public slots:
    void Update();

  private:
    TileViewer* m_tile_viewer;

    Q_OBJECT
};
