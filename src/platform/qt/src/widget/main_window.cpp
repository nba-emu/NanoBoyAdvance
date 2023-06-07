/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <ctime>
#include <fstream>
#include <platform/device/sdl_audio_device.hpp>
#include <platform/loader/bios.hpp>
#include <platform/loader/rom.hpp>
#include <QApplication>
#include <QDateTime>
#include <QMenuBar>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QKeyEvent>
#include <QLocale>
#include <QStatusBar>
#include <unordered_map>

#include "widget/main_window.hpp"

MainWindow::MainWindow(
  QApplication* app,
  QWidget* parent
)   : QMainWindow(parent) {
  setWindowTitle("NanoBoyAdvance 1.7.1");
  setAcceptDrops(true);

  screen = std::make_shared<Screen>(this, config);
  setCentralWidget(screen.get());

  config->Load();

  auto menu_bar = new QMenuBar(this);
  setMenuBar(menu_bar);

  CreateFileMenu();
  CreateConfigMenu();
  CreateHelpMenu();

  config->video_dev = screen;
  config->audio_dev = std::make_shared<nba::SDL2_AudioDevice>();
  config->input_dev = input_device;
  core = nba::CreateCore(config);
  emu_thread = std::make_unique<nba::EmulatorThread>(core);

  app->installEventFilter(this);

  input_window = new InputWindow{app, this, config};
  controller_manager = new ControllerManager(this, config);
  controller_manager->Initialize();

  emu_thread->SetFrameRateCallback([this](float fps) {
    emit UpdateFrameRate(fps);
  });
  connect(this, &MainWindow::UpdateFrameRate, this, [this](int fps) {
    if(config->window.show_fps) {
      auto percent = fps / 59.7275 * 100;
      setWindowTitle(QString::fromStdString(fmt::format("NanoBoyAdvance 1.7.1 [{} fps | {:.2f}%]", fps, percent)));
    } else {
      setWindowTitle("NanoBoyAdvance 1.7.1");
    }
  }, Qt::QueuedConnection);

  UpdateWindowSize();
}

MainWindow::~MainWindow() {
  // Do not let Qt free the screen widget, which must be kept in a
  // shared_ptr right now. This is slightly cursed but oh well.
  (new QMainWindow{})->setCentralWidget(screen.get());

  emu_thread->Stop();

  delete controller_manager;
}

void MainWindow::CreateFileMenu() {
  auto file_menu = menuBar()->addMenu(tr("File"));

  auto open_action = file_menu->addAction(tr("Open"));
  open_action->setShortcut(Qt::CTRL | Qt::Key_O);
  connect(
    open_action,
    &QAction::triggered,
    this,
    &MainWindow::FileOpen
  );

  recent_menu = file_menu->addMenu(tr("Recent"));
  RenderRecentFilesMenu();

  file_menu->addSeparator();

  load_state_menu = file_menu->addMenu(tr("Load state"));
  save_state_menu = file_menu->addMenu(tr("Save state"));
  RenderSaveStateMenus();

  file_menu->addSeparator();

  auto reset_action = file_menu->addAction(tr("Reset"));
  reset_action->setShortcut(Qt::CTRL | Qt::Key_R);
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
  pause_action->setShortcut(Qt::CTRL | Qt::Key_P);
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
    file_menu->addAction(tr("Close")),
    &QAction::triggered,
    &QApplication::quit
  );
}

void MainWindow::CreateVideoMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Video"));

  auto reload_config = [this]() {
    screen->ReloadConfig();
  };

  CreateSelectionOption(menu->addMenu(tr("Filter")), {
    { "Nearest", nba::PlatformConfig::Video::Filter::Nearest },
    { "Linear",  nba::PlatformConfig::Video::Filter::Linear  },
    { "xBRZ",    nba::PlatformConfig::Video::Filter::xBRZ    }
  }, &config->video.filter, false, reload_config);

  CreateSelectionOption(menu->addMenu(tr("Color correction")), {
    { "None",   nba::PlatformConfig::Video::Color::No    },
    { "higan",  nba::PlatformConfig::Video::Color::higan },
    { "GBA",    nba::PlatformConfig::Video::Color::AGB   }
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
  
  auto remap_action = menu->addAction(tr("Configure"));
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

  auto set_save_folder_action = menu->addAction(tr("Set save folder"));
  connect(set_save_folder_action, &QAction::triggered, [this]() {
    SelectSaveFolder();
  });

  auto remove_save_folder_action = menu->addAction(tr("Clear save folder"));
  connect(remove_save_folder_action, &QAction::triggered, [this]() {
    RemoveSaveFolder();
  });

  menu->addSeparator();

  CreateSelectionOption(menu->addMenu(tr("Save type")), {
    { "Detect",      nba::Config::BackupType::Detect },
    { "SRAM",        nba::Config::BackupType::SRAM   },
    { "FLASH 64K",   nba::Config::BackupType::FLASH_64  },
    { "FLASH 128K",  nba::Config::BackupType::FLASH_128 },
    { "EEPROM 512B", nba::Config::BackupType::EEPROM_4  },
    { "EEPROM 8K",   nba::Config::BackupType::EEPROM_64 }
  }, &config->cartridge.backup_type, true);

  CreateBooleanOption(menu, "Force RTC", &config->cartridge.force_rtc, true);
  CreateBooleanOption(menu, "Force solar sensor", &config->cartridge.force_solar_sensor, true);

  menu->addSeparator();

  CreateSolarSensorValueMenu(menu);
}

void MainWindow::CreateSolarSensorValueMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Solar sensor level"));

  current_solar_level = menu->addAction("");
  UpdateSolarSensorLevel(); // needed to update label

  auto SetSolarLevel = [this](int level) {
    config->cartridge.solar_sensor_level = level;
    config->Save();
    UpdateSolarSensorLevel();
  };

  auto CreatePreset = [&](QString const& name, int level) {
    connect(menu->addAction(name), &QAction::triggered, [=]() {
      SetSolarLevel(level);
    });
  };

  menu->addSeparator();

  CreatePreset(tr("Low (23)"), 23);
  CreatePreset(tr("Medium (60)"), 60);
  CreatePreset(tr("High (99)"), 99);
  CreatePreset(tr("Maximum (175)"), 175);

  menu->addSeparator();

  connect(menu->addAction(tr("Enter value...")), &QAction::triggered, [=]() {
    SetSolarLevel(QInputDialog::getInt(
      this,
      tr("Solar sensor level"),
      tr("Enter a value between 0 (lowest intensity) and 255 (highest intensity)"),
      config->cartridge.solar_sensor_level, 0, 255, 1));
  });
}

void MainWindow::CreateWindowMenu(QMenu* parent) {
  auto menu = parent->addMenu(tr("Window"));

  auto scale_menu  = menu->addMenu(tr("Scale"));
  auto scale_group = new QActionGroup{this};

  const auto CreateScaleAction = [&](QString const& label, int scale) {
    auto action = scale_group->addAction(label);

    action->setCheckable(true);
    action->setChecked(config->window.scale == scale);
    action->setShortcut(Qt::SHIFT | (Qt::Key)((int)Qt::Key_1 + scale - 1));

    connect(action, &QAction::triggered, [=]() {
      config->window.scale = scale;
      config->Save();
      UpdateWindowSize();
    });
  };

  auto max_scale_menu = menu->addMenu(tr("Maximum scale"));
  auto max_scale_group = new QActionGroup{this};

  const auto CreateMaximumScaleAction = [&](QString const& label, int scale) {
    auto action = max_scale_group->addAction(label);

    action->setCheckable(true);
    action->setChecked(config->window.maximum_scale == scale);
    action->setShortcut(Qt::ALT | (Qt::Key)((int)Qt::Key_1 + scale - 1));

    connect(action, &QAction::triggered, [=]() {
      config->window.maximum_scale = scale;
      config->Save();
      screen->ReloadConfig();
    });
  };

  CreateMaximumScaleAction(tr("Unlocked"), 0);

  for(int scale = 1; scale <= 8; scale++) {
    auto label = QString::fromStdString(fmt::format("{}x", scale));

    CreateScaleAction(label, scale);
    CreateMaximumScaleAction(label, scale);
  }
  
  scale_menu->addActions(scale_group->actions());
  max_scale_menu->addActions(max_scale_group->actions());

  fullscreen_action = menu->addAction(tr("Fullscreen"));
  fullscreen_action->setCheckable(true);
  fullscreen_action->setChecked(config->window.fullscreen);
#if defined(__APPLE__)
  fullscreen_action->setShortcut(Qt::CTRL | Qt::META | Qt::Key_F);
#else
  fullscreen_action->setShortcut(Qt::CTRL | Qt::Key_F);
#endif
  connect(fullscreen_action, &QAction::triggered, [this](bool fullscreen) {
    SetFullscreen(fullscreen);
  });

#if !defined(__APPLE__)
  CreateBooleanOption(menu, "Show menu in fullscreen", &config->window.fullscreen_show_menu, false, [this]() {
    UpdateMenuBarVisibility();
  })->setShortcut(Qt::CTRL | Qt::Key_M);
#endif

  CreateBooleanOption(menu, "Lock aspect ratio", &config->window.lock_aspect_ratio, false, [this]() {
    screen->ReloadConfig();
  });
  CreateBooleanOption(menu, "Use integer scaling", &config->window.use_integer_scaling, false, [this]() {
    screen->ReloadConfig();
  });

  menu->addSeparator();

  CreateBooleanOption(menu, "Show FPS", &config->window.show_fps);
}

void MainWindow::CreateConfigMenu() {
  auto menu = menuBar()->addMenu(tr("Config"));
  CreateVideoMenu(menu);
  CreateAudioMenu(menu);
  CreateInputMenu(menu);
  CreateSystemMenu(menu);
  CreateWindowMenu(menu);
}

void MainWindow::CreateHelpMenu() {
  auto help_menu = menuBar()->addMenu(tr("Help"));
  auto about_app = help_menu->addAction(tr("About NanoBoyAdvance"));

  about_app->setMenuRole(QAction::AboutRole);
  connect(about_app, &QAction::triggered, [&] {
    QMessageBox box{ this };
    box.setTextFormat(Qt::RichText);
    box.setText(tr("NanoBoyAdvance is a Game Boy Advance emulator focused on accuracy.<br><br>"
                   "Copyright © 2015 - 2023 fleroviux<br><br>"
                   "NanoBoyAdvance is licensed under the GPLv3 or any later version.<br><br>"
                   "GitHub: <a href=\"https://github.com/nba-emu/NanoBoyAdvance\">https://github.com/nba-emu/NanoBoyAdvance</a><br><br>"
                   "Game Boy Advance is a registered trademark of Nintendo Co., Ltd."));
    box.setWindowTitle(tr("About NanoBoyAdvance"));
    box.exec();
  });
}

auto MainWindow::CreateBooleanOption(
  QMenu* menu,
  const char* name,
  bool* underlying,
  bool require_reset,
  std::function<void(void)> callback
) -> QAction* {
  auto action = menu->addAction(QString{name});
  auto config = this->config;

  action->setCheckable(true);
  action->setChecked(*underlying);

  connect(action, &QAction::triggered, [=](bool checked) {
    *underlying = checked;
    config->Save();
    if(require_reset) {
      PromptUserForReset();
    }
    if(callback) {
      callback();
    }
  });

  return action;
}

void MainWindow::RenderRecentFilesMenu() {
  recent_menu->clear();

  size_t i = 0;

  for(auto& path : config->recent_files) {
    auto action = recent_menu->addAction(QString::fromStdString(path));

    action->setShortcut(Qt::CTRL | (Qt::Key) ((int) Qt::Key_0 + i++));

    connect(action, &QAction::triggered, [this, path] {
      LoadROM(QString::fromStdString(path).toStdU16String()); 
    });
  }

  UpdateMainWindowActionList();
}

void MainWindow::RenderSaveStateMenus() {
  load_state_menu->clear();
  save_state_menu->clear();

  for(int i = 1; i <= 10; i++) {
    auto empty_state_name = QString::fromStdString(fmt::format("{:02} - (empty)", i));

    auto action_load = load_state_menu->addAction(empty_state_name);
    auto action_save = save_state_menu->addAction(empty_state_name);
    
    action_load->setDisabled(true);
    action_save->setDisabled(true);

    auto key = (Qt::Key)((int)Qt::Key_F1 + i - 1);
    action_load->setShortcut(key);
    action_save->setShortcut(Qt::SHIFT | key);

    if(game_loaded) {
      auto slot_filename = GetSavePath(game_path, fmt::format("{:02}.nbss", i)).u16string();

      if(fs::exists(slot_filename)) {
        auto file_info = QFileInfo{QString::fromStdU16String(slot_filename)};
        auto date_modified = file_info.lastModified().toLocalTime().toString().toStdString();

        auto state_name = QString::fromStdString(fmt::format("{:02} - {}", i, date_modified));

        action_load->setDisabled(false);
        action_load->setText(state_name);
        action_save->setText(state_name);

        connect(action_load, &QAction::triggered, [=]() {
          if(LoadState(slot_filename) != nba::SaveStateLoader::Result::Success) {
            // The save state may have been deleted, update the save list:
            RenderSaveStateMenus();
          }
        });
      }

      action_save->setDisabled(false);

      connect(action_save, &QAction::triggered, [=]() {
        SaveState(slot_filename);
        RenderSaveStateMenus();
      });
    }
  }

  load_state_menu->addSeparator();
  save_state_menu->addSeparator();

  auto load_file_action = load_state_menu->addAction(tr("From file..."));
  auto save_file_action = save_state_menu->addAction(tr("To file..."));

  load_file_action->setEnabled(game_loaded);
  save_file_action->setEnabled(game_loaded);

  connect(load_file_action, &QAction::triggered, [this]() {
    QFileDialog dialog{};
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("NanoBoyAdvance Save State (*.nbss)");

    if(dialog.exec()) {
      LoadState(dialog.selectedFiles().at(0).toStdU16String());
    }
  });

  connect(save_file_action, &QAction::triggered, [this]() {
    QFileDialog dialog{};
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("NanoBoyAdvance Save State (*.nbss)");

    if(dialog.exec()) {
      SaveState(dialog.selectedFiles().at(0).toStdU16String());
    }
  });

  UpdateMainWindowActionList();
}

void MainWindow::SelectBIOS() {
  QFileDialog dialog{this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("Game Boy Advance BIOS (*.bin)");

  if(dialog.exec()) {
    config->bios_path = dialog.selectedFiles().at(0).toStdString();
    config->Save();
    PromptUserForReset();
  }
}

void MainWindow::SelectSaveFolder() {
  QFileDialog dialog{this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::Directory);

  if(dialog.exec()) {
    config->save_folder = dialog.selectedFiles().at(0).toStdString();
    config->Save();
    PromptUserForReset();
  }
}

void MainWindow::RemoveSaveFolder() {
  config->save_folder = "";
  config->Save();
  PromptUserForReset();
}

void MainWindow::PromptUserForReset() {
  if(emu_thread->IsRunning()) {
    QMessageBox box {this};
    box.setText(tr("The new configuration will apply only after reset.\n\nDo you want to reset the emulation now?"));
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(tr("NanoBoyAdvance"));
    box.addButton(QMessageBox::No);
    box.addButton(QMessageBox::Yes);
    box.setDefaultButton(QMessageBox::No);
    
    if(box.exec() == QMessageBox::Yes) {
      // Reload the ROM in case its config (e.g. save type or GPIO) has changed:
      if(game_loaded) {
        LoadROM(game_path);
      }

      Reset();
    }
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  auto type = event->type();

  if(obj == this && (type == QEvent::KeyPress || type == QEvent::KeyRelease)) {
    auto key = dynamic_cast<QKeyEvent*>(event)->key();
    auto pressed = type == QEvent::KeyPress;
    auto const& input = config->input;

    for(int i = 0; i < nba::InputDevice::kKeyCount; i++) {
      if(input.gba[i].keyboard == key) {
        SetKeyStatus(0, static_cast<nba::InputDevice::Key>(i), pressed);
      }
    }

    if(key == input.fast_forward.keyboard) {
      SetFastForward(0, pressed);
    }

    if(pressed && key == Qt::Key_Escape) {
      SetFullscreen(false);
    }
  } else if(type == QEvent::FileOpen) {
	  auto file = dynamic_cast<QFileOpenEvent*>(event)->file();
    LoadROM(file.toStdU16String());
  } else if(type == QEvent::Drop) {
    const QMimeData* mime_data = ((QDropEvent*)event)->mimeData();

    if(mime_data->hasUrls()) {
      QList<QUrl> url_list = mime_data->urls();

      if(url_list.size() > 0) {
        LoadROM(url_list.at(0).toLocalFile().toStdU16String());
      }
    }
  }

  return QObject::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  event->acceptProposedAction();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    SetFullscreen(false);
  }
}

void MainWindow::FileOpen() {
  QFileDialog dialog {this};
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("Game Boy Advance ROMs (*.gba *.agb *.zip *.rar *.7z *.tar)");

  if(dialog.exec()) {
    auto file = dialog.selectedFiles().at(0).toStdU16String();

    LoadROM(file);
  }
}

void MainWindow::Reset() {
  bool was_running = emu_thread->IsRunning();

  emu_thread->Stop();
  core->Reset();
  if(was_running) {
    emu_thread->Start();
  }
}

void MainWindow::SetPause(bool value) {
  emu_thread->SetPause(value);
  config->audio_dev->SetPause(value);
  pause_action->setChecked(value);
}

void MainWindow::Stop() {
  if(emu_thread->IsRunning()) {
    emu_thread->Stop();
    config->audio_dev->Close();
    screen->SetForceClear(true);

    // Clear the list of save state slots:
    game_loaded = false;
    RenderSaveStateMenus();

    setWindowTitle("NanoBoyAdvance 1.7.1");
  
    UpdateMenuBarVisibility();
  }
}

void MainWindow::UpdateMenuBarVisibility() {
  menuBar()->setVisible(
    !config->window.fullscreen ||
    !emu_thread->IsRunning() ||
     config->window.fullscreen_show_menu
  );

  UpdateMainWindowActionList();
}

void MainWindow::UpdateMainWindowActionList() {
  for(auto action : actions()) {
    removeAction(action);
  }

  if(!menuBar()->isVisible()) {
    for(auto menu : menuBar()->findChildren<QMenu*>()) {
      for(auto action : menu->actions()) {
        if(!action->shortcut().isEmpty()) {
          addAction(action);
        }
      }
    }
  }
}

void MainWindow::LoadROM(std::u16string const& path) {
  bool retry;

  Stop();

  config->Load();

  do {
    retry = false;

    switch(nba::BIOSLoader::Load(core, QString::fromStdString(config->bios_path).toStdU16String())) {
      case nba::BIOSLoader::Result::CannotFindFile: {
        QMessageBox box {this};
        box.setText(tr("A Game Boy Advance BIOS file is required but cannot be located.\n\nWould you like to add one now?"));
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("BIOS not found"));
        box.addButton(QMessageBox::No);
        box.addButton(QMessageBox::Yes);
        box.setDefaultButton(QMessageBox::Yes);
          
        if(box.exec() == QMessageBox::Yes) {
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
  } while(retry);

  auto force_gpio = nba::GPIODeviceType::None;

  if(config->cartridge.force_rtc) {
    force_gpio = force_gpio | nba::GPIODeviceType::RTC;
  }

  if(config->cartridge.force_solar_sensor) {
    force_gpio = force_gpio | nba::GPIODeviceType::SolarSensor;
  }

  auto save_path = GetSavePath(fs::path{path}, ".sav");
  auto save_type = config->cartridge.backup_type;

  auto result = nba::ROMLoader::Load(core, path, save_path, save_type, force_gpio);

  switch(result) {
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

  // Update the list of recent files
  config->UpdateRecentFiles(path);
  RenderRecentFilesMenu();

  // Update the list of save state slots:
  game_loaded = true;
  game_path = path;
  RenderSaveStateMenus();

  // Reset the core and start the emulation thread.
  // Unpause the emulator if it was paused.
  SetPause(false);
  core->Reset();
  emu_thread->Start();
  screen->SetForceClear(false);

  UpdateSolarSensorLevel();
  UpdateMenuBarVisibility();
}

auto MainWindow::LoadState(std::u16string const& path) -> nba::SaveStateLoader::Result {
  bool was_running = emu_thread->IsRunning();
  emu_thread->Stop();

  auto result = nba::SaveStateLoader::Load(core, path);

  QMessageBox box {this};
  box.setIcon(QMessageBox::Critical);

  switch(result) {
    case nba::SaveStateLoader::Result::CannotFindFile:
    case nba::SaveStateLoader::Result::CannotOpenFile: {
      box.setText(tr("Sorry, the save state file could not be opened."));
      box.setWindowTitle(tr("File not found"));
      box.exec();
      break;
    }
    case nba::SaveStateLoader::Result::BadImage: {
      box.setText(tr("Sorry, this save state is corrupted and could not be loaded."));
      box.setWindowTitle(tr("Bad save state"));
      box.exec();
      break;
    }
    case nba::SaveStateLoader::Result::UnsupportedVersion: {
      box.setText(tr("Sorry, this save state was created with a different version of NanoBoyAdvance and could not be loaded."));
      box.setWindowTitle(tr("Unsupported save state version"));
      box.exec();
      break;
    }
    case nba::SaveStateLoader::Result::Success: {
      break;
    }
  }

  if(was_running) {
    emu_thread->Start();
  }

  return result;
}

auto MainWindow::SaveState(std::u16string const& path) -> nba::SaveStateWriter::Result {
  bool was_running = emu_thread->IsRunning();
  emu_thread->Stop();

  auto result = nba::SaveStateWriter::Write(core, path);

  if(result != nba::SaveStateWriter::Result::Success) {
    QMessageBox box {this};
    box.setIcon(QMessageBox::Critical);
    box.setText(tr("Sorry, the save state could not be written to the disk. Make sure that you have sufficient disk space and permissions."));
    box.setWindowTitle(tr("Failed to write to the disk"));
    box.exec();
  }

  if(was_running) {
    emu_thread->Start();
  }

  return result;
}

auto MainWindow::GetSavePath(fs::path const& rom_path, fs::path const& extension) -> fs::path {
  fs::path save_folder = QString::fromStdString(config->save_folder).toStdU16String();

  if(
   !save_folder.empty() &&
    fs::exists(save_folder) &&
    fs::is_directory(save_folder) 
  ) {
    return save_folder / rom_path.filename().replace_extension(extension);
  }

  return fs::path{rom_path}.replace_extension(extension);
}

void MainWindow::SetKeyStatus(int channel, nba::InputDevice::Key key, bool pressed) {
  key_input[channel][int(key)] = pressed;

  input_device->SetKeyStatus(key, 
    key_input[0][int(key)] || key_input[1][int(key)]);
}

void MainWindow::SetFastForward(int channel, bool pressed) {
  auto const& input = config->input;

  fast_forward[channel] = pressed;

  if(input.hold_fast_forward) {
    emu_thread->SetFastForward(fast_forward[0] || fast_forward[1]);
  } else if(!pressed) {
    emu_thread->SetFastForward(!emu_thread->GetFastForward());
  }
}

void MainWindow::UpdateWindowSize() {
  bool fullscreen = config->window.fullscreen;

  if(fullscreen) {
    showFullScreen();
  } else {
    showNormal();

    auto scale = config->window.scale;
    auto minimum_size = screen->minimumSize(); 
    auto maximum_size = screen->maximumSize();
    screen->setFixedSize(240 * scale, 160 * scale);
    adjustSize();
    screen->setMinimumSize(minimum_size);
    screen->setMaximumSize(maximum_size);
  }

  UpdateMenuBarVisibility();
}

void MainWindow::SetFullscreen(bool value) {
  if(config->window.fullscreen != value) {
    config->window.fullscreen = value;
    config->Save();
    UpdateWindowSize();
    fullscreen_action->setChecked(value);
  }
}

void MainWindow::UpdateSolarSensorLevel() {
  auto level = config->cartridge.solar_sensor_level;

  if(core) {
    auto solar_sensor = core->GetROM().GetGPIODevice<nba::SolarSensor>();

    if(solar_sensor) {
      solar_sensor->SetLightLevel(level);
    }
  }

  if(current_solar_level) {
    current_solar_level->setText(
      QString::fromStdString(fmt::format("Current level: {} / 255", level)));
  }
}
