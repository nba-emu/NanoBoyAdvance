/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QLabel>

#include "sprite_viewer.hpp"

class SpriteViewerWindow : public QDialog {
  public:
    SpriteViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  public slots:
    void Update();

  private:
    SpriteViewer* m_sprite_viewer;

    Q_OBJECT
};
