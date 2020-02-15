/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>
#include <QMainWindow>

#include "screen.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QApplication* app, QWidget* parent = 0);
 ~MainWindow();

private:
  std::shared_ptr<Screen> screen;
};