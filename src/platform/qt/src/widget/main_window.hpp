/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <cmath>
#include <functional>
#include <nba/core.hpp>
#include <platform/emulator_thread.hpp>
#include <memory>
#include <QMainWindow>
#include <QActionGroup>
#include <QMenu>
#include <utility>
#include <vector>

#include "widget/controller_manager.hpp"
#include "widget/input_window.hpp"
#include "widget/screen.hpp"
#include "config.hpp"

struct MainWindow : QMainWindow {
  MainWindow(
    QApplication* app,
    QWidget* parent = 0
  );

 ~MainWindow();

  void LoadROM(std::string path);

signals:
  void UpdateFrameRate(int fps);

private slots:
  void FileOpen();

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private:
  friend struct ControllerManager;

  void CreateFileMenu(QMenuBar* menu_bar);
  void CreateVideoMenu(QMenu* parent);
  void CreateAudioMenu(QMenu* parent);
  void CreateInputMenu(QMenu* parent);
  void CreateSystemMenu(QMenu* parent);
  void CreateSolarSensorValueMenu(QMenu* parent);
  void CreateWindowMenu(QMenu* parent);
  void CreateConfigMenu(QMenuBar* menu_bar);
  void CreateHelpMenu(QMenuBar* menu_bar);
  void RenderRecentFilesMenu();

  void SelectBIOS();
  void PromptUserForReset();

  void CreateBooleanOption(
    QMenu* menu,
    const char* name,
    bool* underlying,
    bool require_reset = false,
    std::function<void(void)> callback = nullptr
  );

  template <typename T>
  void CreateSelectionOption(
    QMenu* menu,
    std::vector<std::pair<std::string, T>> const& mapping,
    T* underlying,
    bool require_reset = false,
    std::function<void(void)> callback = nullptr
  ) {
    auto group = new QActionGroup{this};
    auto config = this->config;

    for (auto& entry : mapping) {
      auto action = group->addAction(QString::fromStdString(entry.first));
      action->setCheckable(true);
      action->setChecked(*underlying == entry.second);

      connect(action, &QAction::triggered, [=]() {
        *underlying = entry.second;
        config->Save();
        if (require_reset) {
          PromptUserForReset();
        }
        if (callback) {
          callback();
        }
      });
    }

    menu->addActions(group->actions());
  }

  void Reset();
  void SetPause(bool value);
  void Stop();

  void SetKeyStatus(int channel, nba::InputDevice::Key key, bool pressed);
  void UpdateWindowSize();

  void UpdateSolarSensorLevel();

  std::shared_ptr<Screen> screen;
  std::shared_ptr<nba::BasicInputDevice> input_device = std::make_shared<nba::BasicInputDevice>();
  std::shared_ptr<QtConfig> config = std::make_shared<QtConfig>();
  std::unique_ptr<nba::CoreBase> core;
  std::unique_ptr<nba::EmulatorThread> emu_thread;
  bool key_input[2][nba::InputDevice::kKeyCount] {false};
  ControllerManager* controller_manager;

  QAction* pause_action;
  InputWindow* input_window;
  QMenu* recent_menu;
  QAction* current_solar_level = nullptr;

  nba::SaveState save_state_test;

  Q_OBJECT
};
