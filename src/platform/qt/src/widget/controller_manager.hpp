/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <nba/device/input_device.hpp>
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
  void OnControllerListChanged();
  void OnControllerButtonReleased(int button);
  void OnControllerAxisMoved(int axis, bool negative);

private slots:
  void UpdateGameControllerList();
  void BindCurrentKeyToControllerButton(int button);
  void BindCurrentKeyToControllerAxis(int axis, bool negative);

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