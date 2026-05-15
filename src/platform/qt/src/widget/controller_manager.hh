// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <QTimer>
#include <QWidget>
#include <SDL3/SDL.h>
#include <atomic>
#include <mutex>
#include <thread>

#include "config.hh"

struct MainWindow;

std::string GetJoystickGUIDStringFromIndex(int device_index);

struct ControllerManager : QWidget {
public:
  ControllerManager(MainWindow* main_window, std::shared_ptr<QtConfig> config);
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

  MainWindow* m_main_window;
  std::shared_ptr<QtConfig> m_config;
  std::thread m_thread;
  QTimer* m_timer = nullptr;
  std::atomic_bool m_is_quitting = false;
  SDL_Joystick* m_joystick = nullptr;
  SDL_JoystickID m_instance_id;
  std::mutex m_lock;
  bool m_ff_button_previous_state = false;

  Q_OBJECT
};
