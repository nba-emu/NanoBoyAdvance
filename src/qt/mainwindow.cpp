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

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include <SDL2/SDL.h>

#include "mainwindow.hpp"
#include "settings.hpp"

using namespace Core;

MainWindow::MainWindow(QWidget* parent) :
    m_settings(QSettings::IniFormat, QSettings::UserScope, "flerovium", "NanoboyAdvance"),
    QMainWindow(parent)
{
    setWindowTitle("NanoboyAdvance");

    setupMenu();
    setupScreen();
    setupEmuTimers();
    setupStatusBar();
}

MainWindow::~MainWindow() {
    SDL_Quit();
}

void MainWindow::setupMenu() {
    m_menu_bar = new QMenuBar {this};

    m_file_menu = m_menu_bar->addMenu(tr("&File"));
    m_emul_menu = m_menu_bar->addMenu(tr("&Emulation"));
    m_edit_menu = m_menu_bar->addMenu(tr("Edit"));
    m_help_menu = m_menu_bar->addMenu(tr("&?"));

    // TEST
    connect(m_edit_menu->addAction(tr("Settings")), &QAction::triggered,
        [this] {
            auto ini    = Util::INI("config.ini");
            auto dialog = new SettingsDialog(ini);

            dialog->show();
        }
    );

    setMenuBar(m_menu_bar);

    setupFileMenu();
    setupHelpMenu();
    setupEmulationMenu();
}

void MainWindow::setupFileMenu() {
    m_open_file = m_file_menu->addAction(tr("&Open"));
    m_close     = m_file_menu->addAction(tr("&Close"));

    connect(m_open_file, &QAction::triggered, this, &MainWindow::openGame);
    connect(m_close,     &QAction::triggered, this, &QApplication::quit  );
}

void MainWindow::setupHelpMenu() {
    m_about_app = m_help_menu->addAction(tr("About NanoboyAdvance"));
    m_about_qt  = m_help_menu->addAction(tr("About Qt"));

    m_about_app->setMenuRole(QAction::AboutRole);
    connect(m_about_app, &QAction::triggered,
        [this] {
            QMessageBox::information(this, tr("About NanoboyAdvance"), tr("A small and simple GBA emulator."));
        }
    );

    m_about_qt->setMenuRole(QAction::AboutQtRole);
    connect(m_about_qt, &QAction::triggered, this, &QApplication::aboutQt);
}

void MainWindow::setupEmulationMenu() {
    m_pause_emu = m_emul_menu->addAction(tr("&Pause"));
    m_pause_emu->setCheckable(true);

    m_stop_emu  = m_emul_menu->addAction(tr("&Stop"));

    connect(m_pause_emu, &QAction::triggered, this, &MainWindow::pauseClicked);
    connect(m_stop_emu,  &QAction::triggered, this, &MainWindow::stopClicked );
}

void MainWindow::setupScreen() {
    m_screen = new Screen {this};

    // Key press handler
    connect(m_screen, &Screen::keyPress, this,
        [this] (int key) {
            if (m_emulator == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                m_config->fast_forward = true;
                return;
            }
            m_emulator->setKeyState(MainWindow::qtKeyToEmu(key), true);
        }
    );

    // Key release handler
    connect(m_screen, &Screen::keyRelease, this,
        [this] (int key) {
            if (m_emulator == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                m_config->fast_forward = false;
                return;
            }
            m_emulator->setKeyState(MainWindow::qtKeyToEmu(key), false);
        }
    );

    // Make screen widget our central widget
    setCentralWidget(m_screen);
}

void MainWindow::setupEmuTimers() {
    // Create main timer, ticks at 60Hz
    m_timer = new QTimer {this};
    m_timer->setSingleShot(false);
    m_timer->setInterval(16);

    // Create FPS timer, ticks at 1Hz
    m_timer_fps = new QTimer {this};
    m_timer_fps->setSingleShot(false);
    m_timer_fps->setInterval(1000);

    // Call "nextFrame" each time the main timer ticks
    connect(m_timer, &QTimer::timeout, this, &MainWindow::nextFrame);

    // FPS timer event
    connect(m_timer_fps, &QTimer::timeout, this,
        [this] {
            auto message = std::to_string(m_frames) + " FPS";

            m_frames = 0;
            m_status_msg->setText(QString(message.c_str()));
        }
    );
}

void MainWindow::setupStatusBar() {
    // Create status bar with label text
    m_status_msg = new QLabel     {this};
    m_status_bar = new QStatusBar {this};
    m_status_bar->addPermanentWidget(m_status_msg);

    // Assign status bar to main window
    setStatusBar(m_status_bar);

    // Set a nice default message
    m_status_msg->setText(tr("Idle..."));
}

void MainWindow::nextFrame() {
    // Emulate the next frame(s)
    m_emulator->runFrame();
    m_frames++;

    // Update screen image
    m_screen->updateTexture(m_framebuffer, 240, 160);
}

void MainWindow::openGame() {
    QFileDialog dialog {this};

    dialog.setAcceptMode (QFileDialog::AcceptOpen);
    dialog.setFileMode   (QFileDialog::AnyFile   );
    dialog.setNameFilter ("GameBoyAdvance ROMs (*.gba *.agb)");

    if (dialog.exec()) {
        // Get path of whatever is the first selected file
        QString file = dialog.selectedFiles().at(0);

        // Need to confirm that it actually exists
        if (!QFile::exists(file)) {
            QMessageBox box {this};

            auto dialog_text = tr("Cannot find file ") +
                               QFileInfo(file).fileName();

            box.setText(dialog_text);
            box.setIcon(QMessageBox::Critical);
            box.setWindowTitle(tr("File not found"));

            box.exec();
            return;
        }

        // Load game and start emulating
        runGame(file);
    }
}

void MainWindow::runGame(const QString& rom_file) {
    auto cart = Cartridge::fromFile(rom_file.toStdString());

    const std::string bios_path = "bios.bin";

    if (!QFile::exists(QString(bios_path.c_str()))) {
        QMessageBox box {this};

        box.setText("Please place BIOS as 'bios.bin' in the emulators folder.");
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(tr("BIOS not found."));

        box.exec();
        return;
    }

    // Create config object if not done yet
    if (m_config == nullptr) {
        m_config = new Config();

        m_config->multiplier  = 10;
        m_config->bios_path   = bios_path;
        m_config->framebuffer = m_framebuffer;
    }

    // Create emulator object if not done yet
    if (m_emulator == nullptr) {
        m_emulator = new Emulator(m_config);
    }

    // Apply config and load game.
    m_emulator->reloadConfig();
    m_emulator->loadGame(cart);

    // Setup sound output
    setupSound(&m_emulator->getAPU());

    m_emu_state = EmulationState::Running;

    // Start FPS counting
    m_frames = 0;
    m_timer_fps->start();

    // Start emulating
    m_timer->start();

    m_screen->grabKeyboard();
}

void MainWindow::pauseClicked() {
    switch (m_emu_state) {
        case EmulationState::Paused:
            m_timer->start();
            m_timer_fps->start();
            m_emu_state = EmulationState::Running;
            break;
        case EmulationState::Running:
            m_timer->stop();
            m_timer_fps->stop();
            m_emu_state = EmulationState::Paused;
            break;
    }
}

void MainWindow::stopClicked() {
    m_timer->stop();
    m_timer_fps->stop();
    m_emu_state = EmulationState::Stopped;
}

// Sound callback - called by SDL2. Wraps around C++ method.
void MainWindow::soundCallback(APU* apu, u16* stream, int length) {
    apu->fillBuffer(stream, length);
}

void MainWindow::setupSound(APU* apu) {
    SDL_AudioSpec spec;

    // Setup desired audio spec
    spec.freq     = 44100;
    spec.samples  = 1024;
    spec.format   = AUDIO_U16;
    spec.channels = 2;
    spec.callback = &MainWindow::soundCallback;
    spec.userdata = apu;

    // Initialize SDL2 audio start playing
    SDL_Init(SDL_INIT_AUDIO);
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void MainWindow::closeAudio() {
    SDL_CloseAudioDevice(1);
}

// Translates Qt keycodes to enumeration that Emulator understands
auto MainWindow::qtKeyToEmu(int key) -> Key {
    switch (key) {
        case Qt::Key_A:         return Key::A;
        case Qt::Key_S:         return Key::B;
        case Qt::Key_Backspace: return Key::Select;
        case Qt::Key_Return:    return Key::Start;
        case Qt::Key_Right:     return Key::Right;
        case Qt::Key_Left:      return Key::Left;
        case Qt::Key_Up:        return Key::Up;
        case Qt::Key_Down:      return Key::Down;
        case Qt::Key_W:         return Key::R;
        case Qt::Key_Q:         return Key::L;
        default:                return Key::None;
    }
}
