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
#include <chrono>
#include <QDebug>

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

      static constexpr int kMillisecondsPerSecond = 1000;
      static constexpr int kMicrosecondsPerMillisecond = 1000;

      float frames_per_second = 16777216.0 / 280896.0; // ~ 59.7 fps
      float frame_duration = kMillisecondsPerSecond / frames_per_second; // in milliseconds
      float accumulated_error = 0.0;

      int frames = 0;
      auto timestamp_previous_fps_update = std::chrono::high_resolution_clock::now();

      while (true) {
        auto timestamp_frame_start = std::chrono::high_resolution_clock::now();
        emulator->Frame();
        frames++;
        auto timestamp_frame_end = std::chrono::high_resolution_clock::now();

        auto time_since_last_fps_update = std::chrono::duration_cast<std::chrono::milliseconds>(
          timestamp_frame_end - timestamp_previous_fps_update
        ).count();

        if (time_since_last_fps_update >= kMillisecondsPerSecond) {
          qDebug() << frames << "-" << time_since_last_fps_update;
          frames = 0;
          timestamp_previous_fps_update = timestamp_frame_end;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - timestamp_frame_start
        ).count() / float(kMicrosecondsPerMillisecond);
        
        // NOTE: we need to cast each variable to an integer seperately because
        // we don't want the fractional parts to accumulate and overflow into the integer part.
        auto delay = int(frame_duration) - int(elapsed) + int(accumulated_error);

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        accumulated_error -= int(accumulated_error);
        accumulated_error -= std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now() - timestamp_frame_start
        ).count() - frame_duration;
      }
    });

    test.detach();
  });
}

MainWindow::~MainWindow() {

}