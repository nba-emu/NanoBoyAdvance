/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <cmath>
#include <functional>
#include <filesystem>
#include <nba/core.hpp>
#include <platform/loader/save_state.hpp>
#include <platform/writer/save_state.hpp>
#include <platform/emulator_thread.hpp>
#include <memory>
#include <QMainWindow>
#include <QActionGroup>
#include <QMenu>
#include <utility>
#include <vector>

#include "widget/debugger/ppu/background_viewer_window.hpp"
#include "widget/debugger/ppu/palette_viewer_window.hpp"
#include "widget/debugger/ppu/sprite_viewer_window.hpp"
#include "widget/debugger/ppu/tile_viewer_window.hpp"
#include "widget/controller_manager.hpp"
#include "widget/input_window.hpp"
#include "widget/screen.hpp"
#include "config.hpp"

struct MainWindow : QMainWindow {
  MainWindow(
    QApplication* app,
    QWidget* parent = 0
  );

 ~MainWindow();

  void LoadROM(std::u16string const& path);

signals:
  void UpdateFrameRate(int fps);

private slots:
  void FileOpen();

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  friend struct ControllerManager;

  void CreateFileMenu();
  void CreateVideoMenu(QMenu* parent);
  void CreateAudioMenu(QMenu* parent);
  void CreateInputMenu(QMenu* parent);
  void CreateSystemMenu(QMenu* parent);
  void CreateSolarSensorValueMenu(QMenu* parent);
  void CreateWindowMenu(QMenu* parent);
  void CreateConfigMenu();
  void CreateToolsMenu();
  void CreateHelpMenu();
  void RenderRecentFilesMenu();
  void RenderSaveStateMenus();

  void SelectBIOS();
  void SelectSaveFolder();
  void RemoveSaveFolder();
  void PromptUserForReset();

  auto CreateBooleanOption(
    QMenu* menu,
    const char* name,
    bool* underlying,
    bool require_reset = false,
    std::function<void(void)> callback = nullptr
  ) -> QAction*;

  template <typename T>
  void CreateSelectionOption(
    QMenu* menu,
    std::vector<std::pair<std::string, T>> const& mapping,
    T* underlying,
    bool require_reset = false,
    std::function<void(void)> callback = nullptr
  ) {
    auto group = new QActionGroup{this};
    auto config = this->config;

    for(auto& entry : mapping) {
      auto action = group->addAction(QString::fromStdString(entry.first));
      action->setCheckable(true);
      action->setChecked(*underlying == entry.second);

      connect(action, &QAction::triggered, [=]() {
        *underlying = entry.second;
        config->Save();
        if(require_reset) {
          PromptUserForReset();
        }
        if(callback) {
          callback();
        }
      });
    }

    menu->addActions(group->actions());
  }

  void Reset();
  void SetPause(bool paused);
  void Stop();
  void UpdateMenuBarVisibility();
  void UpdateMainWindowActionList();

  void SetKeyStatus(int channel, nba::Key key, bool pressed);
  void SetFastForward(int channel, bool pressed);
  void UpdateWindowSize();
  void SetFullscreen(bool value);

  void UpdateSolarSensorLevel();

  auto LoadState(std::u16string const& path) -> nba::SaveStateLoader::Result;
  auto SaveState(std::u16string const& path) -> nba::SaveStateWriter::Result;

  auto GetSavePath(fs::path const& rom_path, fs::path const& extension) -> fs::path;

  std::shared_ptr<Screen> screen;
  std::shared_ptr<QtConfig> config = std::make_shared<QtConfig>();
  std::unique_ptr<nba::CoreBase> core;
  std::unique_ptr<nba::EmulatorThread> emu_thread;
  bool key_input[2][(int)nba::Key::Count] {false};
  bool fast_forward[2] {false};
  ControllerManager* controller_manager;

  // The PPU debuggers do not access the core in a thread-safe way yet.
  // So until that is fixed we have to keep a raw pointer around...
  nba::CoreBase* core_not_thread_safe;

  QAction* pause_action;
  InputWindow* input_window;
  QMenu* recent_menu;
  QAction* current_solar_level = nullptr;
  QMenu* load_state_menu;
  QMenu* save_state_menu;
  QAction* fullscreen_action;
  bool game_loaded = false;
  std::u16string game_path;

  nba::SaveState save_state_test;

  PaletteViewerWindow* palette_viewer_window;
  BackgroundViewerWindow* background_viewer_window;
  TileViewerWindow* tile_viewer_window;

  QString base_window_title;

  Q_OBJECT
};
