/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>

#include "palette_viewer.hpp"

class PaletteViewerWindow : public QDialog {
  public:
    PaletteViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  public slots:
    void Update();

  private:
    PaletteViewer* m_palette_viewer;

    Q_OBJECT
};
