/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <atomic>
#include <mutex>
#include <QTimer>
#include <QWidget>
#include <SDL.h>
#include <thread>

#include "config.hpp"
#include "widget/input_window.hpp"

struct MainWindow;

struct ControllerManager : QWidget {
  ControllerManager(
    MainWindow* main_window,
    std::shared_ptr<QtConfig> config
  )   : main_window(main_window)
      , config(config) {
  }

 ~ControllerManager();

  void Initialize();

signals:
  void OnJoystickListChanged();
  void OnJoystickButtonReleased(int button);
  void OnJoystickAxisMoved(int axis, bool negative);
  void OnJoystickHatMoved(int hat, int direction);

private slots:
  void UpdateJoystickList();
  void BindCurrentKeyToJoystickButton(int button);
  void BindCurrentKeyToJoystickAxis(int axis, bool negative);
  void BindCurrentKeyToJoystickHat(int hat, int direction);

private:
  void Open(std::string const& guid);
  void Close();
  void ProcessEvents();
  void UpdateKeyState();

  MainWindow* main_window;
  std::shared_ptr<QtConfig> config;
  std::thread thread;
  QTimer* timer = nullptr;
  std::atomic_bool quitting = false;
  SDL_Joystick* joystick = nullptr;
  SDL_JoystickID instance_id;
  std::mutex lock;
  bool fast_forward_button_old = false;

  Q_OBJECT
};