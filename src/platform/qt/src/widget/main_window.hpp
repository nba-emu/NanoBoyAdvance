/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <nba/core.hpp>
#include <platform/emulator_thread.hpp>
#include <memory>
#include <QMainWindow>
#include <QActionGroup>
#include <QMenu>
#include <SDL.h>
#include <utility>
#include <vector>

#include "widget/input_window.hpp"
#include "widget/screen.hpp"
#include "config.hpp"

struct MainWindow : QMainWindow {
  MainWindow(
    QApplication* app,
    QWidget* parent = 0
  );

 ~MainWindow();

signals:
  void UpdateFrameRate(int fps);

private slots:
  void FileOpen();

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private:
  static constexpr auto kConfigPath = "config.toml";

  void CreateFileMenu(QMenuBar* menu_bar);
  void CreateOptionsMenu(QMenuBar* menu_bar);
  void CreateHelpMenu(QMenuBar* menu_bar);
  void CreateBooleanOption(QMenu* menu, const char* name, bool* underlying);

  template <typename T>
  void CreateSelectionOption(QMenu* menu, std::vector<std::pair<std::string, T>> const& mapping, T* underlying) {
    auto group = new QActionGroup{this};
    auto config = this->config;

    for (auto& entry : mapping) {
      auto action = group->addAction(QString::fromStdString(entry.first));
      action->setCheckable(true);
      action->setChecked(*underlying == entry.second);

      connect(action, &QAction::triggered, [=]() {
        *underlying = entry.second;
        config->Save(kConfigPath);
      });
    }

    menu->addActions(group->actions());
  }

  void FindGameController();
  void UpdateGameControllerInput();
  void UpdateWindowSize();

  std::shared_ptr<Screen> screen;
  std::shared_ptr<nba::BasicInputDevice> input_device = std::make_shared<nba::BasicInputDevice>();
  std::shared_ptr<QtConfig> config = std::make_shared<QtConfig>();
  std::unique_ptr<nba::CoreBase> core;
  std::unique_ptr<nba::EmulatorThread> emu_thread;

  InputWindow* input_window;

  SDL_GameController* game_controller = nullptr;
  bool game_controller_button_x_old = false;

  Q_OBJECT
};
