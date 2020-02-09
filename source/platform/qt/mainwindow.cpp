/**
  * Copyright (C) 2020 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

// TEST
#include <gba/emulator.hpp>
#include <thread>

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
    //auto config = std::make_shared<GameBoyAdvance::Config>();
    //config->video_dev = screen;

    //auto emulator = std::make_unique<GameBoyAdvance::Emulator>(config);
    //emulator->LoadGame("violet.gba");
    //emulator->Reset();
    //for (int i = 0; i < 20000; i++) {
    //  emulator->Frame();
    //}

    std::thread test([&] {
      auto config = std::make_shared<GameBoyAdvance::Config>();
      config->video_dev = screen;

      auto emulator = std::make_unique<GameBoyAdvance::Emulator>(config);
      emulator->LoadGame("violet.gba");
      emulator->Reset();

      while (true) {
        emulator->Frame();
      }
    });

    test.detach();
  });
}

MainWindow::~MainWindow() {

}