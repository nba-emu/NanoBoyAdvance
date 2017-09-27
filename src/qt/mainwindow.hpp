/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QSettings>

#include "screen.hpp"
#include "core/emulator/emulator.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
     MainWindow(QWidget* parent = 0);
    ~MainWindow();

public slots:
    // Shows Open ROM dialog
    void openGame();

    void nextFrame();

private:
    enum class EmulationState {
        Stopped,
        Running,
        Paused
    };

    void setupMenu();
    void setupFileMenu();
    void setupHelpMenu();
    void setupEmulationMenu();
    void setupScreen();
    void setupEmuTimers();
    void setupStatusBar();

    void runGame(const QString& rom_file);
    void pauseClicked();
    void stopClicked();

    // Setup SDL2 Audio Subsystem
    void setupSound(Core::APU* apu);
    void closeAudio();

    // Called by SDL2 to request new audio
    static void soundCallback(Core::APU* apu, u16* stream, int length);

    static auto qtKeyToEmu(int key) -> Core::Key;

    QSettings m_settings;

    QMenuBar* m_menu_bar;

    // Menus
    QMenu* m_file_menu;
    QMenu* m_emul_menu;
    QMenu* m_edit_menu;
    QMenu* m_help_menu;
    
    // Menu Actions
    QAction* m_open_file;
    QAction* m_pause_emu;
    QAction* m_stop_emu;
    QAction* m_close;
    QAction* m_open_settings;
    QAction* m_about_qt;
    QAction* m_about_app;

    // Timer
    QTimer* m_timer;
    QTimer* m_timer_fps;

    // Program Status
    QLabel*     m_status_msg;
    QStatusBar* m_status_bar;

    // Display widget
    Screen* m_screen;

    int m_frames {0};
    u32 m_framebuffer[240 * 160];

    // Keep track of emulation state: Running/Stopped/Paused
    EmulationState m_emu_state { EmulationState::Stopped };

    // Emulator instance
    Core::Config*   m_config   { nullptr };
    Core::Emulator* m_emulator { nullptr };
};
