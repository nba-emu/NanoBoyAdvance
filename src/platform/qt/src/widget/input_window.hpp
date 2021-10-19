/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <filesystem>
#include <fstream>
#include <nba/device/input_device.hpp>
#include <nba/log.hpp>
#include <toml.hpp>
#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPushButton>
#include <QLabel>

#include "config.hpp"

struct InputWindow : QDialog {
  using Key = nba::InputDevice::Key;

  InputWindow(
    QApplication* app,
    QWidget* parent,
    std::shared_ptr<QtConfig> config
  );

  std::shared_ptr<QtConfig> config;

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private:
  void CreateKeyMapEntry(
    QGridLayout* layout,
    const char* label,
    int* key
  );

  static auto GetKeyName(int key) -> QString;

  bool waiting_for_keypress = false;
  int* current_key = nullptr;
  QPushButton* current_button = nullptr;
};