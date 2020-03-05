/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

// TEST
#include <emulator/emulator.hpp>
#include <thread>
#include <QDebug>
#include <common/framelimiter.hpp>

#include <platform/sdl/device/audio_device.hpp>
#include <QApplication>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>

#include "mainwindow.hpp"

MainWindow::MainWindow(QApplication* app, QWidget* parent) : QMainWindow(parent) {
  setWindowTitle("NanoboyAdvance");
  app->installEventFilter(this);

  /* Setup OpenGL widget */
  screen = std::make_shared<Screen>(this);
  setCentralWidget(screen.get());

  /* Create menu bar */
  auto menubar = new QMenuBar(this);
  setMenuBar(menubar);

  /* Create file menu */
  auto file_menu = menubar->addMenu(tr("&File"));
  auto file_open = file_menu->addAction(tr("&Open"));
  auto file_close = file_menu->addAction(tr("&Close"));

  /* Create help menu */
  auto help_menu = menubar->addMenu(tr("&?"));
  auto about_app = help_menu->addAction(tr("About NanoboyAdvance"));
  auto about_qt  = help_menu->addAction(tr("About Qt"));

  /* Set emulator config */
  config->video_dev = screen;
  config->audio_dev = std::make_shared<SDL2_AudioDevice>();
  config->input_dev = input_device;
  emulator = std::make_unique<nba::Emulator>(config);

  connect(file_open, &QAction::triggered, this, &MainWindow::FileOpen);
  connect(file_close, &QAction::triggered, &QApplication::quit);
  about_app->setMenuRole(QAction::AboutRole);
  connect(about_app, &QAction::triggered, [&]{
    QMessageBox box {this};
    box.setTextFormat(Qt::RichText);
    box.setText(tr("NanoboyAdvance is a portable Game Boy Advance emulator.<br><br>"
                   "Copyright (C) 2018 - 2020 Frederic Raphael Meyer (fleroviux)<br><br>"
                   "NanoboyAdvance is licensed under the GPLv3 or any later version.<br><br>"
                   "Github: <a href=\"https://github.com/fleroviux/NanoboyAdvance\">https://github.com/fleroviux/NanoboyAdvance</a><br>"
                   "Twitter: <a href=\"https://twitter.com/fleroviux\">@fleroviux</a><br><br>"
                   "Game Boy Advance is a registered trademark of Nintendo Co., Ltd."));
    box.setWindowTitle(tr("About NanoboyAdvance"));
    box.exec();
  });
  about_qt->setMenuRole(QAction::AboutQtRole);
  connect(about_qt, &QAction::triggered, &QApplication::aboutQt);

  app->installEventFilter(this);
}

MainWindow::~MainWindow() {

}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  auto type = event->type();

  if (type != QEvent::KeyPress && type != QEvent::KeyRelease) {
    return QObject::eventFilter(obj, event);
  }

  auto key = dynamic_cast<QKeyEvent*>(event)->key();
  auto pressed = type == QEvent::KeyPress;

  /* TODO: support dynamic key mapping. */
  switch (key) {
    case Qt::Key_Y:
    case Qt::Key_Z: {
      input_device->SetKeyStatus(nba::InputDevice::Key::A, pressed);
      return true;
    }
    case Qt::Key_X: {
      input_device->SetKeyStatus(nba::InputDevice::Key::B, pressed);
      return true;
    }
    case Qt::Key_Return: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Start, pressed);
      return true;
    }
    case Qt::Key_Backspace: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Select, pressed);
      return true;
    }
    case Qt::Key_Up: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Up, pressed);
      return true;
    }
    case Qt::Key_Down: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Down, pressed);
      return true;
    }
    case Qt::Key_Left: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Left, pressed);
      return true;
    }
    case Qt::Key_Right: {
      input_device->SetKeyStatus(nba::InputDevice::Key::Right, pressed);
      return true;
    }
    case Qt::Key_Space: {
      framelimiter.Unbounded(pressed);
      return true;
    }
    default: {
      return QObject::eventFilter(obj, event);
    }
  }
}

void MainWindow::FileOpen() {
  using StatusCode = nba::Emulator::StatusCode;

  QFileDialog dialog {this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setNameFilter("GameBoyAdvance ROMs (*.gba *.agb)");

  if (!dialog.exec()) {
    return;
  }

  auto file = dialog.selectedFiles().at(0);

  if (emulator_state == EmulationState::Running) {
    emulator_state = EmulationState::Stopped;
    while (emulator_thread_running) ;
  }

  /* TODO: move message boxes to another method. */
  switch (emulator->LoadGame(file.toStdString())) {
    case StatusCode::GameNotFound: {
      QMessageBox box {this};
      box.setText(tr("Cannot find file: ") + QFileInfo(file).fileName());
      box.setIcon(QMessageBox::Critical);
      box.setWindowTitle(tr("File not found"));
      box.exec();
      return;
    }
    case StatusCode::BiosNotFound: {
      QMessageBox box {this};
      box.setText(tr("Please reference a BIOS file in the configuration.\n\n"
                     "Cannot open BIOS: ") + QString{config->bios_path.c_str()});
      box.setIcon(QMessageBox::Critical);
      box.setWindowTitle(tr("BIOS not found"));
      box.exec();
      return;
    }
    case StatusCode::GameWrongSize: {
      QMessageBox box {this};
      box.setIcon(QMessageBox::Critical);
      box.setText(tr("The file you opened exceeds the maximum size of 32 MiB."));
      box.setWindowTitle(tr("ROM exceeds maximum size"));
      box.exec();
      return;
    }
    case StatusCode::BiosWrongSize: {
      QMessageBox box {this};
      box.setIcon(QMessageBox::Critical);
      box.setText(tr("Your BIOS file exceeds the maximum size of 16 KiB."));
      box.setWindowTitle(tr("BIOS file exceeds maximum size"));
      box.exec();
      return;
    }
  }

  emulator->Reset();
  emulator_state = EmulationState::Running;

  emulator_thread = std::thread([this] {
    emulator_thread_running = true;

    framelimiter.Reset(); // foo

    while (emulator_state == EmulationState::Running) {
      framelimiter.Run([&] {
        emulator->Frame();
      }, [&](int fps) {
        this->setWindowTitle(QString{ (std::string("NanoboyAdvance [") + std::to_string(fps) + std::string(" fps]")).c_str() });
      });
    }

    emulator_thread_running = false;
  });

  emulator_thread.detach();
}