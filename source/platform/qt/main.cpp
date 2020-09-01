/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>
#include <QApplication>

#include "mainwindow.hpp"

int main(int argc, char** argv) {
  QApplication app{ argc, argv };
  MainWindow window{ &app };

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoboyAdvance");

  common::logger::init();

  window.show();
  return app.exec();
}
