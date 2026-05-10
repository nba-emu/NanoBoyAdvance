// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <QDialog>
#include <QLabel>

#include "sprite_viewer.hh"

class SpriteViewerWindow : public QDialog {
  public:
    SpriteViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  public slots:
    void Update();

  private:
    SpriteViewer* m_sprite_viewer;

    Q_OBJECT
};
