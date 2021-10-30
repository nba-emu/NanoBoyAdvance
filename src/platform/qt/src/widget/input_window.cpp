/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "widget/input_window.hpp"

InputWindow::InputWindow(
  QApplication* app,
  QWidget* parent,
  std::shared_ptr<QtConfig> config
)   : QDialog(parent)
    , config(config) {
  auto layout = new QGridLayout{this};
  layout->setSizeConstraint(QLayout::SetFixedSize);

  CreateKeyMapEntry(layout, "A", &config->input.gba[int(Key::A)]);
  CreateKeyMapEntry(layout, "B", &config->input.gba[int(Key::B)]);
  CreateKeyMapEntry(layout, "L", &config->input.gba[int(Key::L)]);
  CreateKeyMapEntry(layout, "R", &config->input.gba[int(Key::R)]);
  CreateKeyMapEntry(layout, "Start", &config->input.gba[int(Key::Start)]);
  CreateKeyMapEntry(layout, "Select", &config->input.gba[int(Key::Select)]);
  CreateKeyMapEntry(layout, "Up", &config->input.gba[int(Key::Up)]);
  CreateKeyMapEntry(layout, "Down", &config->input.gba[int(Key::Down)]);
  CreateKeyMapEntry(layout, "Left", &config->input.gba[int(Key::Left)]);
  CreateKeyMapEntry(layout, "Right", &config->input.gba[int(Key::Right)]);
  CreateKeyMapEntry(layout, "Fast Forward", &config->input.fast_forward);

  app->installEventFilter(this);

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

bool InputWindow::eventFilter(QObject* obj, QEvent* event) {
  if (waiting_for_keypress && event->type() == QEvent::KeyPress) {
    auto key_event = dynamic_cast<QKeyEvent*>(event);
    auto key = key_event->key();
    auto name = QKeySequence{key_event->key()}.toString();
    *current_key = key;
    current_button->setText(GetKeyName(key));
    waiting_for_keypress = false;
    config->Save("config.toml");
    return true;
  }

  return QObject::eventFilter(obj, event);
}

void InputWindow::CreateKeyMapEntry(
  QGridLayout* layout,
  const char* label,
  int* key
) {
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

auto InputWindow::GetKeyName(int key) -> QString {
  if (key == 0) {
    return "None";
  }
  return QKeySequence{key}.toString();
}