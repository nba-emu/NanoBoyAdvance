/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QPainter>

struct BackgroundViewerWindow : QDialog {
  BackgroundViewerWindow(nba::CoreBase* core, QWidget* parent = nullptr);

  bool eventFilter(QObject* object, QEvent* event);

public slots:
  void Update();

private:
  void DrawBackground();
  void DrawBackgroundMode0(QPainter& painter, int bg_id);
  void DrawBackgroundMode2(QPainter& painter, int bg_id);
  void DrawBackgroundMode3(QPainter& painter);
  void DrawBackgroundMode4(QPainter& painter);
  void DrawBackgroundMode5(QPainter& painter);

  nba::CoreBase* core;
  u16* pram;
  u8* vram;

  QLabel* test_label;
  QWidget* canvas;
  QImage image{1024, 1024, QImage::Format_RGB555};

  Q_OBJECT
};
