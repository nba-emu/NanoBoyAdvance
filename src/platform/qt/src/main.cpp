/*
 * Copyright (C) 2026 Mireille Meyer
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QApplication>
#include <QSurfaceFormat>
#include <QProxyStyle>
#include <cstdlib>
#include <filesystem>
#include <memory>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN 1
//#define NOMINMAX 1 // msys defines this already

#include <Windows.h>
#endif

#include "widget/main_window.hpp"

namespace fs = std::filesystem;

// See: https://stackoverflow.com/a/37023032
struct MenuStyle : QProxyStyle {
  int styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget, QStyleHintReturn *returnData) const {
    if(stylehint == QStyle::SH_MenuBar_AltKeyNavigation) return 0;

    return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
  }
};

auto create_window(QApplication& app, int argc, char** argv) -> std::unique_ptr<MainWindow> {
  fs::path rom;

  if(argc >= 2) {
    rom = fs::path{argv[1]};

    if(rom.is_relative()) {
      rom = fs::current_path() / rom;
    }

    fs::path binary_directory = fs::path{argv[0]}.remove_filename();
    if(binary_directory.is_relative()) {
      binary_directory = fs::current_path() / binary_directory;
    }
    fs::current_path(binary_directory);
  }

  auto window = std::make_unique<MainWindow>(&app);
  if(!window->Initialize()) {
    return nullptr;
  }

  if(!rom.empty()) {
    window->LoadROM(rom.u16string());
  }

  window->show();
  return window;
}

int main(int argc, char** argv) {
#if defined(WIN32)
  constexpr auto terminal_output = "CONOUT$";
  constexpr auto null_output = "NUL:";

  // Check whether we are already attached to an output stream.
  const auto output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if(!output_handle || (output_handle == INVALID_HANDLE_VALUE)) {
    // If started from terminal, attach and output logs to it.
    if(AttachConsole(ATTACH_PARENT_PROCESS)) {
      if(!freopen(terminal_output, "a", stdout)) { assert(false); }
      if(!freopen(terminal_output, "a", stderr)) { assert(false); }
    } else {
      // Otherwise, discard log output.
      if(!freopen(null_output, "w", stdout)) { assert(false); }
      if(!freopen(null_output, "w", stderr)) { assert(false); }
    }
  }
#endif

  // On some systems (e.g. macOS) QSurfaceFormat::setDefaultFormat() must be called before constructing a QApplication.
  auto format = QSurfaceFormat{};
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(3);
  format.setMinorVersion(3);
  format.setSwapInterval(0);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app{ argc, argv };

  app.setStyle(new MenuStyle());

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoBoyAdvance");
  QGuiApplication::setDesktopFileName("io.nanoboyadvance.NanoBoyAdvance");

  auto window = create_window(app, argc, argv);
  if(!window) {
    return EXIT_FAILURE;
  }

  return app.exec();
}
