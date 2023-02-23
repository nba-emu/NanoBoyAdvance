/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <memory>
#include <QApplication>
#include <QSurfaceFormat>
#include <QProxyStyle>
#include <stdlib.h>

#include "widget/main_window.hpp"

#if defined(WIN32)
  #include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

namespace fs = std::filesystem;

// See: https://stackoverflow.com/a/37023032
struct MenuStyle : QProxyStyle {
  int styleHint(
    StyleHint stylehint,
    const QStyleOption *opt,
    const QWidget *widget,
    QStyleHintReturn *returnData
  ) const {
    if (stylehint == QStyle::SH_MenuBar_AltKeyNavigation)
      return 0;

    return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
  }
};

auto create_window(QApplication& app, int argc, char** argv) -> std::unique_ptr<MainWindow> {
  fs::path rom;

  if (argc >= 2) {
    rom = fs::path{argv[1]};

    if (rom.is_relative()) {
      rom = fs::current_path() / rom;
    }
    fs::current_path(fs::path{argv[0]}.remove_filename());
  }

  auto window = std::make_unique<MainWindow>(&app);

  if (!rom.empty()) {
    window->LoadROM(rom.string());
  }

  window->show();
  return window;
}

int main(int argc, char** argv) {
  // See: https://trac.wxwidgets.org/ticket/19023
#if defined(__APPLE__)
  setenv("LC_NUMERIC", "C", 1);
#endif

  // On some systems (e.g. macOS) QSurfaceFormat::setDefaultFormat() must be called before constructing QApplication.
  auto format = QSurfaceFormat{};
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(3);
  format.setMinorVersion(3);
  format.setSwapInterval(0);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app{ argc, argv };

  app.setStyle(new MenuStyle());

#if defined(WIN32)
  // See: https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
#endif

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoBoyAdvance");

  auto window = create_window(app, argc, argv);

  return app.exec();
}
