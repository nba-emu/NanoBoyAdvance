/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QLabel>

#include "widget/sprite_viewer.hpp"

struct SpriteViewerWindow : QDialog {
  SpriteViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

public slots:
  void Update();

private:
  SpriteViewer* sprite_viewer;

  Q_OBJECT
};
