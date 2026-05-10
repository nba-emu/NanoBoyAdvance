// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

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
