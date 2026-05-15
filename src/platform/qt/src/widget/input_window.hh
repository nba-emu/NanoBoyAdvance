// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <toml.hpp>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <atomic>

#include "config.hh"

struct InputWindow : QDialog {
  using Key = nba::Key;

  InputWindow(QApplication* app, QWidget* parent, std::shared_ptr<QtConfig> config);

  void BindCurrentKeyToJoystickButton(int button);
  void BindCurrentKeyToJoystickAxis(int axis, bool negative);
  void BindCurrentKeyToJoystickHat(int hat, int direction);

  void UpdateJoystickList();

  std::atomic_bool has_game_controller_choice_changed = false;

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private:
  auto CreateJoystickList() -> QLayout*;
  auto CreateKeyMapTable() -> QLayout*;

  void CreateKeyMapEntry(
    QGridLayout* layout,
    const char* label,
    QtConfig::Input::Map* mapping
  );

  void RestoreActiveButtonLabel();

  static auto GetKeyboardButtonName(int key) -> QString;
  static auto GetJoystickButtonName(QtConfig::Input::Map* mapping) -> QString;

  bool waiting_for_keyboard = false;
  bool waiting_for_joystick = false;
  QtConfig::Input::Map* active_mapping = nullptr;
  QPushButton* active_button = nullptr;
  QComboBox* joystick_combo_box;
  std::shared_ptr<QtConfig> config;
};
