/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QApplication>
#include <stdlib.h>

#include "widget/main_window.hpp"

int main(int argc, char** argv) {
  // See: https://trac.wxwidgets.org/ticket/19023
#if defined(__APPLE__)
  setenv("LC_NUMERIC", "C", 1);
#endif

  QApplication app{ argc, argv };
  MainWindow window{ &app };

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoBoyAdvance");

  window.show();
  return app.exec();
}
