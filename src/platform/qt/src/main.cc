// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QSurfaceFormat>
#include <QProxyStyle>
#include <cstdlib>
#include <filesystem>
#include <memory>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1

#include <Windows.h>
#endif

#include <atom/logger/sink/console.hh>
#include <atom/panic.hh>
#include <QMessageBox>

#include "widget/main_window.hh"

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

static void atom_panic_handler(const char* file, int line, const char* message);

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

#if defined(linux)
  // Fix for https://github.com/nba-emu/NanoBoyAdvance/issues/425
  // Qt 6, Wayland and EGL currently don't seem to get along very well, causing QOpenGLContext::create to fail with EGL_BAD_MATCH (3009) or EGL_BAD_SURFACE (300d).
  // We're unfortunately not the only emulator plagued by this (https://github.com/mgba-emu/mgba/issues/3736, https://github.com/melonDS-emu/melonDS/issues/2643).
  // To alleviate this problem, we simply run with Xcb instead of Wayland, which is also what Valve's SteamOS does, and their contractors at least get paid to know stuff. I sure don't!
  // This should be removed once the problem is solved.
  setenv("QT_QPA_PLATFORM", "xcb", 0);
#endif

  // Atom setup
  atom::get_logger().InstallSink(std::make_shared<atom::LoggerConsoleSink>());
  atom::set_panic_handler(atom_panic_handler);

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

static void atom_panic_handler(const char* file, int line, const char* message) {
  QMessageBox::critical(nullptr, "Panic", QStringLiteral("%2\n\nFile: %0:%1").arg(file).arg(line).arg(message));
}
