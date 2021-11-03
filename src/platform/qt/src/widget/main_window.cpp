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

  screen = std::make_shared<Screen>(this, config);
  setCentralWidget(screen.get());

  config->Load(kConfigPath);

  auto menu_bar = new QMenuBar(this);
  setMenuBar(menu_bar);

  CreateFileMenu(menu_bar);
  CreateConfigMenu(menu_bar);
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
  // Do not let Qt free the screen widget, which must be kept in a
  // shared_ptr right now. This is slightly cursed but oh well.
  (new QMainWindow{})->setCentralWidget(screen.get());

  emu_thread->Stop();

  if (game_controller != nullptr) {
    SDL_GameControllerClose(game_controller);
  }
}

void MainWindow::CreateFileMenu(QMenuBar* menu_bar) {
  auto file_menu = menu_bar->addMenu(tr("&File"));

  auto open_action = file_menu->addAction(tr("&Open"));
  open_action->setShortcut(Qt::CTRL + Qt::Key_O);
  connect(
    open_action,
    &QAction::triggered,
    this,
    &MainWindow::FileOpen
  );

  file_menu->addSeparator();

  auto reset_action = file_menu->addAction(tr("Reset"));
  reset_action->setShortcut(Qt::CTRL + Qt::Key_R);
  connect(
    reset_action,
    &QAction::triggered,
    [this]() {
      Reset();
    }
  );

  pause_action = file_menu->addAction(tr("Pause"));
  pause_action->setCheckable(true);
  pause_action->setChecked(false);
  pause_action->setShortcut(Qt::CTRL + Qt::Key_P);
  connect(
    pause_action,
    &QAction::triggered,
    [this](bool paused) {
      SetPause(paused);
    }
  );

  connect(
    file_menu->addAction(tr("Stop")),
    &QAction::triggered,
    [this]() {
      Stop();
    }
  );

  file_menu->addSeparator();

  connect(
    file_menu->addAction(tr("&Close")),
    &QAction::triggered,
    &QApplication::quit
  );
}

void MainWindow::CreateVideoMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Video"));
  auto scale_menu  = menu->addMenu(tr("Scale"));
  auto scale_group = new QActionGroup{this};

  for (int scale = 1; scale <= 6; scale++) {
    auto action = scale_group->addAction(QString::fromStdString(fmt::format("{}x", scale)));

    action->setCheckable(true);
    action->setChecked(config->video.scale == scale);

    connect(action, &QAction::triggered, [=]() {
      config->video.scale = scale;
      config->Save(kConfigPath);
      UpdateWindowSize();
    });
  }
  
  scale_menu->addActions(scale_group->actions());

  auto fullscreen_action = menu->addAction(tr("Fullscreen"));
  fullscreen_action->setCheckable(true);
  fullscreen_action->setChecked(config->video.fullscreen);
  connect(fullscreen_action, &QAction::triggered, [this](bool fullscreen) {
    config->video.fullscreen = fullscreen;
    config->Save(kConfigPath);
    UpdateWindowSize();
  });

  menu->addSeparator();

  auto reload_config = [this]() {
    screen->ReloadConfig();
  };

  CreateSelectionOption(menu->addMenu(tr("Filter")), {
    { "Nearest", nba::PlatformConfig::Video::Filter::Nearest },
    { "Linear",  nba::PlatformConfig::Video::Filter::Linear  },
    { "xBRZ",    nba::PlatformConfig::Video::Filter::xBRZ    }
  }, &config->video.filter, false, reload_config);

  CreateSelectionOption(menu->addMenu(tr("Color correction")), {
    { "None",   nba::PlatformConfig::Video::Color::No  },
    { "GBA",    nba::PlatformConfig::Video::Color::AGB },
    { "GBA SP", nba::PlatformConfig::Video::Color::AGS },
  }, &config->video.color, false, reload_config);

  CreateBooleanOption(menu, "LCD ghosting", &config->video.lcd_ghosting, false, reload_config);
}

void MainWindow::CreateAudioMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Audio"));

  CreateSelectionOption(menu->addMenu("Resampler"), {
    { "Cosine",   nba::Config::Audio::Interpolation::Cosine },
    { "Cubic",    nba::Config::Audio::Interpolation::Cubic  },
    { "Sinc-64",  nba::Config::Audio::Interpolation::Sinc_64  },
    { "Sinc-128", nba::Config::Audio::Interpolation::Sinc_128 },
    { "Sinc-256", nba::Config::Audio::Interpolation::Sinc_256 }
  }, &config->audio.interpolation, true);

  auto hq_menu = menu->addMenu("MP2K HQ mixer");
  CreateBooleanOption(hq_menu, "Enable", &config->audio.mp2k_hle_enable, true);
  CreateBooleanOption(hq_menu, "Cubic interpolation", &config->audio.mp2k_hle_cubic, true);
}

void MainWindow::CreateInputMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Input"));
  
  auto remap_action = menu->addAction(tr("Remap"));
  remap_action->setMenuRole(QAction::NoRole);
  connect(remap_action, &QAction::triggered, [this] {
    input_window->exec();
  });

  CreateBooleanOption(menu, "Hold fast forward key", &config->input.hold_fast_forward);
}

void MainWindow::CreateSystemMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("System"));

  auto bios_path_action = menu->addAction(tr("Set BIOS path"));
  connect(bios_path_action, &QAction::triggered, [this] {
    SelectBIOS();
  });

  CreateBooleanOption(menu, "Skip BIOS", &config->skip_bios);

  menu->addSeparator();

  CreateSelectionOption(menu->addMenu(tr("Save Type")), {
    { "Detect",      nba::Config::BackupType::Detect },
    { "SRAM",        nba::Config::BackupType::SRAM   },
    { "FLASH 64K",   nba::Config::BackupType::FLASH_64  },
    { "FLASH 128K",  nba::Config::BackupType::FLASH_128 },
    { "EEPROM 512B", nba::Config::BackupType::EEPROM_4  },
    { "EEPROM 8K",   nba::Config::BackupType::EEPROM_64 }
  }, &config->backup_type, true);

  CreateBooleanOption(menu, "Force RTC", &config->force_rtc, true);
}

void MainWindow::CreateConfigMenu(QMenuBar* menu_bar) {
  auto menu = menu_bar->addMenu(tr("&Config"));
  CreateVideoMenu(menu);
  CreateAudioMenu(menu);
  CreateInputMenu(menu);
  CreateSystemMenu(menu);
}

void MainWindow::CreateBooleanOption(
  QMenu* menu,
  const char* name,
  bool* underlying,
  bool require_reset,
  std::function<void(void)> callback
) {
  auto action = menu->addAction(QString{name});
  auto config = this->config;

  action->setCheckable(true);
  action->setChecked(*underlying);

  connect(action, &QAction::triggered, [=](bool checked) {
    *underlying = checked;
    config->Save(kConfigPath);
    if (require_reset) {
      PromptUserForReset();
    }
    if (callback) {
      callback();
    }
  });
}

void MainWindow::CreateHelpMenu(QMenuBar* menu_bar) {
  auto help_menu = menu_bar->addMenu(tr("&Help"));
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

void MainWindow::SelectBIOS() {
  QFileDialog dialog{this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("Game Boy Advance BIOS (*.bin)");

  if (dialog.exec()) {
    config->bios_path = dialog.selectedFiles().at(0).toStdString();
    config->Save(kConfigPath);
  }
}

void MainWindow::PromptUserForReset() {
  if (emu_thread->IsRunning()) {
    QMessageBox box {this};
    box.setText(tr("The new configuration will apply only after reset.\n\nDo you want to reset the emulation now?"));
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(tr("NanoBoyAdvance"));
    box.addButton(QMessageBox::No);
    box.addButton(QMessageBox::Yes);
    box.setDefaultButton(QMessageBox::No);
    
    if (box.exec() == QMessageBox::Yes) {
      Reset();
    }
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  auto type = event->type();

  if (obj == this && (type == QEvent::KeyPress || type == QEvent::KeyRelease)) {
    auto key = dynamic_cast<QKeyEvent*>(event)->key();
    auto pressed = type == QEvent::KeyPress;
    auto const& input = config->input;

    for (int i = 0; i < nba::InputDevice::kKeyCount; i++) {
      if (input.gba[i] == key) {
        input_device->SetKeyStatus(static_cast<nba::InputDevice::Key>(i), pressed);
      }
    }

    if (key == input.fast_forward) {
      if (input.hold_fast_forward) {
        emu_thread->SetFastForward(pressed);
      } else if (!pressed) {
        emu_thread->SetFastForward(!emu_thread->GetFastForward());
      }
    }
  }

  return QObject::eventFilter(obj, event);
}

void MainWindow::FileOpen() {
  QFileDialog dialog {this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("Game Boy Advance ROMs (*.gba *.agb)");

  if (dialog.exec()) {
    bool retry;
    auto file = dialog.selectedFiles().at(0);

    emu_thread->Stop();
    config->Load(kConfigPath);

    do {
      retry = false;

      switch (nba::BIOSLoader::Load(core, config->bios_path)) {
        case nba::BIOSLoader::Result::CannotFindFile: {
          QMessageBox box {this};
          box.setText(tr("A Game Boy Advance BIOS file is required but cannot be located.\n\nWould you like to add one now?"));
          box.setIcon(QMessageBox::Question);
          box.setWindowTitle(tr("BIOS not found"));
          box.addButton(QMessageBox::No);
          box.addButton(QMessageBox::Yes);
          box.setDefaultButton(QMessageBox::Yes);
          
          if (box.exec() == QMessageBox::Yes) {
            SelectBIOS();
            retry = true;
            continue;
          }
          return;
        }
        case nba::BIOSLoader::Result::CannotOpenFile:
        case nba::BIOSLoader::Result::BadImage: {
          QMessageBox box {this};
          box.setText(tr("Sorry, the BIOS file could not be loaded.\n\nMake sure that the BIOS image is valid and has correct file permissions."));
          box.setIcon(QMessageBox::Critical);
          box.setWindowTitle(tr("Cannot open BIOS"));
          box.exec();
          return;
        }
      }
    } while (retry);

    switch (nba::ROMLoader::Load(core, file.toStdString(), config->backup_type, config->force_rtc)) {
      case nba::ROMLoader::Result::CannotFindFile: {
        QMessageBox box {this};
        box.setText(tr("Sorry, the specified ROM file cannot be located."));
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(tr("ROM not found"));
        box.exec();
        return;
      }
      case nba::ROMLoader::Result::CannotOpenFile:
      case nba::ROMLoader::Result::BadImage: {
        QMessageBox box {this};
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Sorry, the ROM file could not be loaded.\n\nMake sure that the ROM image is valid and has correct file permissions."));
        box.setWindowTitle(tr("Cannot open ROM"));
        box.exec();
        return;
      }
    }

    core->Reset();
    emu_thread->Start();
  }
}

void MainWindow::Reset() {
  bool was_running = emu_thread->IsRunning();

  emu_thread->Stop();
  core->Reset();
  if (was_running) {
    emu_thread->Start();
  }
}

void MainWindow::SetPause(bool value) {
  emu_thread->SetPause(value);
  config->audio_dev->SetPause(value);
  pause_action->setChecked(value);
}

void MainWindow::Stop() {
  if (emu_thread->IsRunning()) {
    emu_thread->Stop();
    config->audio_dev->Close();
    screen->Clear();

    setWindowTitle("NanoBoyAdvance 1.4");
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