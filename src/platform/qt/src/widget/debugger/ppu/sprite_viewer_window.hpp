/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
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
