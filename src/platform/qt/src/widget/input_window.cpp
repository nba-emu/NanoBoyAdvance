/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <SDL.h>

#include "widget/input_window.hpp"

InputWindow::InputWindow(
  QApplication* app,
  QWidget* parent,
  std::shared_ptr<QtConfig> config
)   : QDialog(parent)
    , config(config) {
  auto vbox = new QVBoxLayout{this};
  vbox->setSizeConstraint(QLayout::SetFixedSize);
  vbox->addLayout(CreateGameControllerList());
  vbox->addLayout(CreateKeyMapTable());

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
    config->Save();
    return true;
  }

  return QObject::eventFilter(obj, event);
}

auto InputWindow::CreateGameControllerList() -> QLayout* {
  auto hbox = new QHBoxLayout{};

  controller_combo_box = new QComboBox();

  auto label = new QLabel{"Game Controller:"};
  hbox->addWidget(label);
  hbox->addWidget(controller_combo_box);

  connect(controller_combo_box, QOverload<int>::of(&QComboBox::activated), [this](int index) {
    config->input.controller_guid = controller_combo_box->itemData(index).toString().toStdString();
    config->Save();

    has_game_controller_choice_changed = true;
  });

  UpdateGameControllerList();
  return hbox;
}

void InputWindow::UpdateGameControllerList() {
  auto joystick_count = SDL_NumJoysticks();

  controller_combo_box->clear();

  controller_combo_box->addItem("(none)", "");
  controller_combo_box->setCurrentIndex(0);

  for (int i = 0; i < joystick_count; i++) {
    if (SDL_IsGameController(i)) {
      auto guid = GetControllerGUIDStringFromIndex(i);

      controller_combo_box->addItem(SDL_GameControllerNameForIndex(i), QString::fromStdString(guid));

      if (guid == config->input.controller_guid) {
        controller_combo_box->setCurrentIndex(controller_combo_box->count() - 1);
      }
    }
  }
}

auto InputWindow::CreateKeyMapTable() -> QLayout* {
  auto grid = new QGridLayout{this};

  CreateKeyMapEntry(grid, "A", &config->input.gba[int(Key::A)].keyboard);
  CreateKeyMapEntry(grid, "B", &config->input.gba[int(Key::B)].keyboard);
  CreateKeyMapEntry(grid, "L", &config->input.gba[int(Key::L)].keyboard);
  CreateKeyMapEntry(grid, "R", &config->input.gba[int(Key::R)].keyboard);
  CreateKeyMapEntry(grid, "Start", &config->input.gba[int(Key::Start)].keyboard);
  CreateKeyMapEntry(grid, "Select", &config->input.gba[int(Key::Select)].keyboard);
  CreateKeyMapEntry(grid, "Up", &config->input.gba[int(Key::Up)].keyboard);
  CreateKeyMapEntry(grid, "Down", &config->input.gba[int(Key::Down)].keyboard);
  CreateKeyMapEntry(grid, "Left", &config->input.gba[int(Key::Left)].keyboard);
  CreateKeyMapEntry(grid, "Right", &config->input.gba[int(Key::Right)].keyboard);
  CreateKeyMapEntry(grid, "Fast Forward", &config->input.fast_forward.keyboard);
  return grid;
}

void InputWindow::CreateKeyMapEntry(
  QGridLayout* layout,
  const char* label,
  int* key
) {
  auto row = layout->rowCount();
  auto button = new QPushButton{GetKeyName(*key)};

  layout->addWidget(new QLabel{label}, row, 0);
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