/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <common/framelimiter.hpp>
#include <emulator/emulator.hpp>
#include <memory>
#include <QMainWindow>

#include "screen.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QApplication* app, QWidget* parent = 0);
 ~MainWindow();

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private slots:
  void FileOpen();

private:
  enum class EmulationState {
    Stopped,
    Running,
    Paused
  } emulator_state = EmulationState::Stopped;

  std::atomic<bool> emulator_thread_running;

  std::shared_ptr<Screen> screen;
  std::shared_ptr<nba::BasicInputDevice> input_device = std::make_shared<nba::BasicInputDevice>();
  std::shared_ptr<nba::Config> config = std::make_shared<nba::Config>();
  std::unique_ptr<nba::Emulator> emulator;
  common::Framelimiter framelimiter {59.7275};
  std::thread emulator_thread;
};