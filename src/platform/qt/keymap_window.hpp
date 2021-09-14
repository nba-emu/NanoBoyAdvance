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

struct KeyMap {
  int keypad[nba::InputDevice::kKeyCount] {
    Qt::Key_Up,
    Qt::Key_Down,
    Qt::Key_Left,
    Qt::Key_Right,
    Qt::Key_Return,
    Qt::Key_Backspace,
    Qt::Key_A,
    Qt::Key_S,
    Qt::Key_D,
    Qt::Key_F
  };

  int fast_forward = Qt::Key_Space;

  // TODO: keep keymap in the config.toml file.

  void Load() {
    toml::value data;

    try {
      data = toml::parse("keymap.toml");
    } catch (std::exception& ex) {
      nba::Log<nba::Warn>("Qt: failed to load keymap configuration.");
      return;
    }

    if (data.contains("general")) {
      auto general_result = toml::expect<toml::value>(data.at("general"));

      if (general_result.is_ok()) {
        fast_forward = toml::find_or<int>(general_result.unwrap(), "fast_forward", Qt::Key_Space);
      }
    }

    if (data.contains("gba")) {
      auto gba_result = toml::expect<toml::value>(data.at("gba"));

      if (gba_result.is_ok()) {
        auto gba = gba_result.unwrap();

        // TODO: refactor this to be less ugly...
        keypad[0] = toml::find_or<int>(gba, "up", Qt::Key_Up);
        keypad[1] = toml::find_or<int>(gba, "down", Qt::Key_Down);
        keypad[2] = toml::find_or<int>(gba, "left", Qt::Key_Left);
        keypad[3] = toml::find_or<int>(gba, "right", Qt::Key_Right);
        keypad[4] = toml::find_or<int>(gba, "start", Qt::Key_Return);
        keypad[5] = toml::find_or<int>(gba, "select", Qt::Key_Backspace);
        keypad[6] = toml::find_or<int>(gba, "a", Qt::Key_A);
        keypad[7] = toml::find_or<int>(gba, "b", Qt::Key_S);
        keypad[8] = toml::find_or<int>(gba, "l", Qt::Key_D);
        keypad[9] = toml::find_or<int>(gba, "r", Qt::Key_F);
      }
    }
  }

  void Save() {
    toml::basic_value<toml::preserve_comments> data;

    if (std::filesystem::exists("keymap.toml")) {
      try {
        data = toml::parse<toml::preserve_comments>("keymap.toml");
      } catch (std::exception& ex) {
        nba::Log<nba::Error>("Qt: failed to parse keymap configuration: {}", ex.what());
        return;
      }
    }

    // General
    data["general"]["fast_forward"] = fast_forward;

    // GBA keypad
    // TODO: refactor this to be less ugly...
    data["gba"]["up"] = keypad[0];
    data["gba"]["down"] = keypad[1];
    data["gba"]["left"] = keypad[2];
    data["gba"]["right"] = keypad[3];
    data["gba"]["start"] = keypad[4];
    data["gba"]["select"] = keypad[5];
    data["gba"]["a"] = keypad[6];
    data["gba"]["b"] = keypad[7];
    data["gba"]["l"] = keypad[8];
    data["gba"]["r"] = keypad[9];

    std::ofstream file{ "keymap.toml", std::ios::out };
    file << data;
    file.close();
  }
};

struct KeyMapWindow : QDialog {
  KeyMapWindow(QApplication* app, QWidget* parent) : QDialog(parent) {
    auto layout = new QGridLayout{this};

    using Key = nba::InputDevice::Key;

    map.Load();

    CreateKeyMapEntry(layout, "A", &map.keypad[int(Key::A)]);
    CreateKeyMapEntry(layout, "B", &map.keypad[int(Key::B)]);
    CreateKeyMapEntry(layout, "L", &map.keypad[int(Key::L)]);
    CreateKeyMapEntry(layout, "R", &map.keypad[int(Key::R)]);
    CreateKeyMapEntry(layout, "Start", &map.keypad[int(Key::Start)]);
    CreateKeyMapEntry(layout, "Select", &map.keypad[int(Key::Select)]);
    CreateKeyMapEntry(layout, "Up", &map.keypad[int(Key::Up)]);
    CreateKeyMapEntry(layout, "Down", &map.keypad[int(Key::Down)]);
    CreateKeyMapEntry(layout, "Left", &map.keypad[int(Key::Left)]);
    CreateKeyMapEntry(layout, "Right", &map.keypad[int(Key::Right)]);
    CreateKeyMapEntry(layout, "Fast Forward", &map.fast_forward);

    app->installEventFilter(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  }

  KeyMap map;

protected:
  bool eventFilter(QObject* obj, QEvent* event) {
    auto type = event->type();

    if (type != QEvent::KeyPress) {
      return QObject::eventFilter(obj, event);
    }

    if (waiting_for_keypress) {
      auto key_event = dynamic_cast<QKeyEvent*>(event);
      auto key = key_event->key();
      auto name = QKeySequence{key_event->key()}.toString();
      *current_key = key;
      current_button->setText(GetKeyName(key));
      waiting_for_keypress = false;
      map.Save();
      return true;
    }

    return QObject::eventFilter(obj, event);
  }

private:
  void CreateKeyMapEntry(QGridLayout* layout, const char* label, int* key) {
    auto row = layout->rowCount();
    auto button = new QPushButton{GetKeyName(*key), this};

    layout->addWidget(new QLabel{label, this}, row, 0);
    layout->addWidget(button, row, 1);

    connect(button, &QPushButton::clicked, [=]() {
      if (waiting_for_keypress) {
        current_button->setText(GetKeyName(*current_key));
      }
      button->setText("[press key]");
      current_key = key;
      current_button = button;
      waiting_for_keypress = true;
    });
  }

  static auto GetKeyName(int key) -> QString {
    if (key == 0) {
      return "None";
    }
    return QKeySequence{key}.toString();
  }

  bool waiting_for_keypress = false;
  int* current_key = nullptr;
  QPushButton* current_button = nullptr;
};