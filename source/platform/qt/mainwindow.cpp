/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

// TEST
#include <gba/emulator.hpp>
#include <thread>
#include <QDebug>
#include <common/framelimiter.hpp>

#include <QApplication>
#include <QMenuBar>

#include "mainwindow.hpp"

MainWindow::MainWindow(QApplication* app, QWidget* parent) : QMainWindow(parent) {
  setWindowTitle("NanoboyAdvance");
  app->installEventFilter(this);

  /* Setup OpenGL widget */
  screen = std::make_shared<Screen>(this);
  setCentralWidget(screen.get());

  /* Create menu bar */
  auto menubar = new QMenuBar(this);
  setMenuBar(menubar);

  /* Create file menu */
  auto file_menu = menubar->addMenu(tr("&File"));
  auto file_open = file_menu->addAction(tr("&Open"));
  auto file_close = file_menu->addAction(tr("&Close"));

  /* Create help menu */
  auto help_menu = menubar->addMenu(tr("&?"));

  connect(file_open, &QAction::triggered, this, [this] {
    std::thread test([&] {
      auto config = std::make_shared<GameBoyAdvance::Config>();
      config->video_dev = screen;

      auto emulator = std::make_unique<GameBoyAdvance::Emulator>(config);
      emulator->LoadGame("violet.gba");
      emulator->Reset();

      Common::Framelimiter framelimiter;

      framelimiter.Reset(16777216.0 / 280896.0); // ~ 59.7 fps

      while (true) {
        framelimiter.Run([&] {
          emulator->Frame();
        }, [&](int fps) {
          this->setWindowTitle(QString{ (std::string("NanoboyAdvance [") + std::to_string(fps) + std::string(" fps]")).c_str() });
        });
      }
    });

    test.detach();
  });
}

MainWindow::~MainWindow() {

}