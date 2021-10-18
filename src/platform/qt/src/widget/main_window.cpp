/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <platform/device/sdl_audio_device.hpp>
#include <platform/loader/bios.hpp>
#include <platform/loader/rom.hpp>
#include <QApplication>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QStatusBar>
#include <unordered_map>

#include "widget/main_window.hpp"

MainWindow::MainWindow(
  QApplication* app,
  QWidget* parent
)   : QMainWindow(parent) {
  setWindowTitle("NanoBoyAdvance 1.4");

  screen = std::make_shared<Screen>(this);
  setCentralWidget(screen.get());

  config->Load(kConfigPath);

  auto menu_bar = new QMenuBar(this);
  menu_bar->setNativeMenuBar(false);
  setMenuBar(menu_bar);

  CreateFileMenu(menu_bar);
  CreateOptionsMenu(menu_bar);
  CreateHelpMenu(menu_bar);

  config->video_dev = screen;
  config->audio_dev = std::make_shared<nba::SDL2_AudioDevice>();
  config->input_dev = input_device;
  core = nba::CreateCore(config);
  emu_thread = std::make_unique<nba::EmulatorThread>(core);

  app->installEventFilter(this);

  input_window = new InputWindow{app, this, config};

  FindGameController();

  emu_thread->SetFrameRateCallback([this](float fps) {
    emit UpdateFrameRate(fps);
  });
  connect(this, &MainWindow::UpdateFrameRate, this, [this](int fps) {
    auto percent = fps / 59.7275 * 100;
    setWindowTitle(QString::fromStdString(fmt::format("NanoBoyAdvance 1.4 [{} fps | {:.2f}%]", fps, percent)));;
  }, Qt::BlockingQueuedConnection);

  UpdateWindowSize();
}

MainWindow::~MainWindow() {
  emu_thread->Stop();

  if (game_controller != nullptr) {
    SDL_GameControllerClose(game_controller);
  }
}

void MainWindow::CreateFileMenu(QMenuBar* menu_bar) {
  auto file_menu = menu_bar->addMenu(tr("&File"));
  auto file_open = file_menu->addAction(tr("&Open"));
  auto file_close = file_menu->addAction(tr("&Close"));

  connect(file_open, &QAction::triggered, this, &MainWindow::FileOpen);
  connect(file_close, &QAction::triggered, &QApplication::quit);
}

void MainWindow::CreateOptionsMenu(QMenuBar* menu_bar) {
  auto options_menu = menu_bar->addMenu(tr("&Options"));

  auto options_bios_menu = options_menu->addMenu(tr("BIOS"));
  auto options_bios_path = options_bios_menu->addAction(tr("Select path"));
  CreateBooleanOption(options_bios_menu, "Skip intro", &config->skip_bios);
  connect(options_bios_path, &QAction::triggered, [this] {
    QFileDialog dialog{this};
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Game Boy Advance BIOS (*.bin)");

    if (!dialog.exec()) {
      return;
    }

    config->bios_path = dialog.selectedFiles().at(0).toStdString();
    config->Save(kConfigPath);
  });

  auto options_cartridge_menu = options_menu->addMenu(tr("Cartridge"));
  CreateBooleanOption(options_cartridge_menu, "Force RTC", &config->force_rtc);
  CreateSelectionOption(options_cartridge_menu->addMenu(tr("Save Type")), {
    { "Detect",      nba::Config::BackupType::Detect },
    { "SRAM",        nba::Config::BackupType::SRAM   },
    { "FLASH 64K",   nba::Config::BackupType::FLASH_64  },
    { "FLASH 128K",  nba::Config::BackupType::FLASH_128 },
    { "EEPROM 512B", nba::Config::BackupType::EEPROM_4  },
    { "EEPROM 8K",   nba::Config::BackupType::EEPROM_64 }
  }, &config->backup_type);

  auto options_video_menu = options_menu->addMenu(tr("Video"));
  auto options_video_scale_menu = options_video_menu->addMenu(tr("Scale"));
  auto options_video_scale_menu_group = new QActionGroup{this};
  for (int scale = 1; scale <= 6; scale++) {
    auto action = options_video_scale_menu_group->addAction(QString::fromStdString(fmt::format("{}x", scale)));

    action->setCheckable(true);
    action->setChecked(config->video.scale == scale);

    connect(action, &QAction::triggered, [=]() {
      config->video.scale = scale;
      config->Save(kConfigPath);
      UpdateWindowSize();
    });
  }
  options_video_scale_menu->addActions(options_video_scale_menu_group->actions());
  options_video_menu->addSeparator();
  auto fullscreen_action = options_video_menu->addAction(tr("Fullscreen"));
  fullscreen_action->setCheckable(true);
  fullscreen_action->setChecked(config->video.fullscreen);
  connect(fullscreen_action, &QAction::triggered, [this](bool fullscreen) {
    config->video.fullscreen = fullscreen;
    config->Save(kConfigPath);
    UpdateWindowSize();
  });

  auto options_audio_menu = options_menu->addMenu(tr("Audio"));
  CreateSelectionOption(options_audio_menu->addMenu("Resampler"), {
    { "Cosine",   nba::Config::Audio::Interpolation::Cosine },
    { "Cubic",    nba::Config::Audio::Interpolation::Cubic  },
    { "Sinc-64",  nba::Config::Audio::Interpolation::Sinc_64  },
    { "Sinc-128", nba::Config::Audio::Interpolation::Sinc_128 },
    { "Sinc-256", nba::Config::Audio::Interpolation::Sinc_256 }
  }, &config->audio.interpolation);

  auto options_hq_audio_menu = options_audio_menu->addMenu("HQ audio mixer");
  CreateBooleanOption(options_hq_audio_menu, "Enable", &config->audio.mp2k_hle_enable);
  CreateBooleanOption(options_hq_audio_menu, "Use cubic filter", &config->audio.mp2k_hle_cubic);

  auto configure_input = options_menu->addAction(tr("Configure input"));
  connect(configure_input, &QAction::triggered, [this] {
    input_window->exec();
  });
}

void MainWindow::CreateBooleanOption(QMenu* menu, const char* name, bool* underlying) {
  auto action = menu->addAction(QString{name});
  auto config = this->config;

  action->setCheckable(true);
  action->setChecked(*underlying);

  connect(action, &QAction::triggered, [=](bool checked) {
    *underlying = checked;
    config->Save(kConfigPath);
  });
}

void MainWindow::CreateHelpMenu(QMenuBar* menu_bar) {
  auto help_menu = menu_bar->addMenu(tr("&?"));
  auto about_app = help_menu->addAction(tr("About NanoBoyAdvance"));

  about_app->setMenuRole(QAction::AboutRole);
  connect(about_app, &QAction::triggered, [&] {
    QMessageBox box{ this };
    box.setTextFormat(Qt::RichText);
    box.setText(tr("NanoBoyAdvance is a Game Boy Advance emulator with a focus on high accuracy.<br><br>"
                   "Copyright © 2015 - 2021 fleroviux<br><br>"
                   "NanoBoyAdvance is licensed under the GPLv3 or any later version.<br><br>"
                   "GitHub: <a href=\"https://github.com/fleroviux/NanoBoyAdvance\">https://github.com/fleroviux/NanoBoyAdvance</a><br><br>"
                   "Game Boy Advance is a registered trademark of Nintendo Co., Ltd."));
    box.setWindowTitle(tr("About NanoBoyAdvance"));
    box.exec();
  });
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  auto type = event->type();

  if (type != QEvent::KeyPress && type != QEvent::KeyRelease) {
    return QObject::eventFilter(obj, event);
  }

  auto key = dynamic_cast<QKeyEvent*>(event)->key();
  auto pressed = type == QEvent::KeyPress;

  for (int i = 0; i < nba::InputDevice::kKeyCount; i++) {
    if (config->input.gba[i] == key) {
      input_device->SetKeyStatus(static_cast<nba::InputDevice::Key>(i), pressed);
    }
  }

  if (key == config->input.fast_forward) {
    emu_thread->SetFastForward(pressed);
  }

  return QObject::eventFilter(obj, event);
}

void MainWindow::FileOpen() {
  QFileDialog dialog {this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("Game Boy Advance ROMs (*.gba *.agb)");

  if (dialog.exec()) {
    auto file = dialog.selectedFiles().at(0);

    emu_thread->Stop();

    config->Load(kConfigPath);

    switch (nba::BIOSLoader::Load(core, config->bios_path)) {
      case nba::BIOSLoader::Result::CannotFindFile: {
        // TODO: ask the user if they want to assign the file.
        QMessageBox box {this};
        box.setText(tr("Please reference a BIOS file in the configuration.\n\n"
                       "Cannot find BIOS: ") + QString{config->bios_path.c_str()});
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(tr("BIOS not found"));
        box.exec();
        return;
      }
      case nba::BIOSLoader::Result::CannotOpenFile:
      case nba::BIOSLoader::Result::BadImage: {
        QMessageBox box {this};
        box.setText(tr("Failed to open the BIOS file. "
                       "Make sure that the permissions are correct and that the BIOS image is valid.\n\n"
                       "Cannot open BIOS: ") + QString{config->bios_path.c_str()});
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(tr("Cannot open BIOS"));
        box.exec();
        return;
      }
    }

    switch (nba::ROMLoader::Load(core, file.toStdString(), config->backup_type, config->force_rtc)) {
      case nba::ROMLoader::Result::CannotFindFile: {
        QMessageBox box {this};
        box.setText(tr("Cannot find file: ") + QFileInfo(file).fileName());
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(tr("File not found"));
        box.exec();
        return;
      }
      case nba::ROMLoader::Result::CannotOpenFile:
      case nba::ROMLoader::Result::BadImage: {
        QMessageBox box {this};
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("The file could not be opened. Make sure that the permissions are correct and that the ROM image is valid."));
        box.setWindowTitle(tr("Cannot open ROM"));
        box.exec();
        return;
      }
    }

    core->Reset();
    emu_thread->Start();
  }
}

void MainWindow::FindGameController() {
  SDL_Init(SDL_INIT_GAMECONTROLLER);

  auto num_joysticks = SDL_NumJoysticks();

  for (int i = 0; i < num_joysticks; i++) {
    if (SDL_IsGameController(i)) {
      game_controller = SDL_GameControllerOpen(i);
      if (game_controller != nullptr) {
        nba::Log<nba::Info>("Qt: detected game controller '{0}'", SDL_GameControllerNameForIndex(i));
        break;
      }
    }
  }
}

void MainWindow::UpdateGameControllerInput() {
  using Key = nba::InputDevice::Key;

  if (game_controller == nullptr) {
    return;
  }

  SDL_GameControllerUpdate();

  auto button_x = SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_X);
  if (game_controller_button_x_old && !button_x) {
    emu_thread->SetFastForward(!emu_thread->GetFastForward());
  }
  game_controller_button_x_old = button_x;

  static const std::unordered_map<SDL_GameControllerButton, Key> buttons{
    { SDL_CONTROLLER_BUTTON_A, Key::A },
    { SDL_CONTROLLER_BUTTON_B, Key::B },
    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER , Key::L },
    { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Key::R },
    { SDL_CONTROLLER_BUTTON_START, Key::Start },
    { SDL_CONTROLLER_BUTTON_BACK, Key::Select }
  };

  for (auto& button : buttons) {
    if (SDL_GameControllerGetButton(game_controller, button.first)) {
      input_device->SetKeyStatus(button.second, true);
    } else {
      input_device->SetKeyStatus(button.second, false);
    }
  }

  constexpr auto threshold = std::numeric_limits<int16_t>::max() / 2;
  auto x = SDL_GameControllerGetAxis(game_controller, SDL_CONTROLLER_AXIS_LEFTX);
  auto y = SDL_GameControllerGetAxis(game_controller, SDL_CONTROLLER_AXIS_LEFTY);

  input_device->SetKeyStatus(Key::Left, x < -threshold);
  input_device->SetKeyStatus(Key::Right, x > threshold);
  input_device->SetKeyStatus(Key::Up, y < -threshold);
  input_device->SetKeyStatus(Key::Down, y > threshold);
}

void MainWindow::UpdateWindowSize() {
  if (config->video.fullscreen) {
    showFullScreen();
  } else {
    showNormal();

    auto scale = config->video.scale;
    auto minimum_size = screen->minimumSize();
    auto maximum_size = screen->maximumSize();
    screen->setFixedSize(240 * scale, 160 * scale);
    adjustSize();
    screen->setMinimumSize(minimum_size);
    screen->setMaximumSize(maximum_size);
  }
}