/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QApplication>

#include "widget/main_window.hpp"

int main(int argc, char** argv) {
  QApplication app{ argc, argv };
  MainWindow window{ &app };

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoBoyAdvance");

  window.show();
  return app.exec();
}
