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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
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
    menubar.bar = new QMenuBar {this};

    menubar.file.menu      = menubar.bar->addMenu(tr("&File"));
    menubar.emulation.menu = menubar.bar->addMenu(tr("&Emulation"));
    menubar.help.menu      = menubar.bar->addMenu(tr("&?"));

    setMenuBar(menubar.bar);

    setupFileMenu();
    setupHelpMenu();
    setupEmulationMenu();
}

void MainWindow::setupFileMenu() {
    auto& file = menubar.file;

    file.open  = file.menu->addAction(tr("&Open"));
    file.close = file.menu->addAction(tr("&Close"));

    connect(file.open,  &QAction::triggered, this, &MainWindow::openGame);
    connect(file.close, &QAction::triggered, this, &QApplication::quit  );
}

void MainWindow::setupHelpMenu() {
    auto& help = menubar.help;

    help.app = help.menu->addAction(tr("About NanoboyAdvance"));
    help.qt  = help.menu->addAction(tr("About Qt"));

    help.app->setMenuRole(QAction::AboutRole);
    connect(help.app, &QAction::triggered,
        [this] {
            QMessageBox::information(this, tr("About NanoboyAdvance"), tr("A small and simple GBA emulator."));
        }
    );

    help.qt->setMenuRole(QAction::AboutQtRole);
    connect(help.qt, &QAction::triggered, this, &QApplication::aboutQt);
}

void MainWindow::setupEmulationMenu() {
    auto& emulation = menubar.emulation;

    emulation.pause = emulation.menu->addAction(tr("&Pause"));
    emulation.stop  = emulation.menu->addAction(tr("&Stop"));

    emulation.pause->setCheckable(true);

    connect(emulation.pause, &QAction::triggered, this, &MainWindow::pauseClicked);
    connect(emulation.stop,  &QAction::triggered, this, &MainWindow::stopClicked );
}

void MainWindow::setupScreen() {
    screen = new Screen {this};

    // Key press handler
    connect(screen, &Screen::keyPress, this,
        [this] (int key) {
            if (emulator == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                config->fast_forward = true;
                return;
            }
            emulator->setKeyState(MainWindow::qtKeyToEmu(key), true);
        }
    );

    // Key release handler
    connect(screen, &Screen::keyRelease, this,
        [this] (int key) {
            if (emulator == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                config->fast_forward = false;
                return;
            }
            emulator->setKeyState(MainWindow::qtKeyToEmu(key), false);
        }
    );

    // Make screen widget our central widget
    setCentralWidget(screen);
}

void MainWindow::setupEmuTimers() {
    // Create main timer, ticks at 60Hz
    timer_run = new QTimer {this};
    timer_run->setSingleShot(false);
    timer_run->setInterval(16);

    // Create FPS timer, ticks at 1Hz
    timer_fps = new QTimer {this};
    timer_fps->setSingleShot(false);
    timer_fps->setInterval(1000);

    // Call "nextFrame" each time the main timer ticks
    connect(timer_run, &QTimer::timeout, this, &MainWindow::nextFrame);

    // FPS timer event
    connect(timer_fps, &QTimer::timeout, this,
        [this] {
            auto message = std::to_string(frames) + " FPS";

            frames = 0;
            status_msg->setText(QString(message.c_str()));
        }
    );
}

void MainWindow::setupStatusBar() {
    // Create status bar with label text
    status_msg = new QLabel     {this};
    status_bar = new QStatusBar {this};
    status_bar->addPermanentWidget(status_msg);

    // Assign status bar to main window
    setStatusBar(status_bar);

    // Set a nice default message
    status_msg->setText(tr("Idle..."));
}

void MainWindow::nextFrame() {
    // Emulate the next frame(s)
    emulator->runFrame();
    frames++;

    // Update screen image
    screen->updateTexture(framebuffer, 240, 160);
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
    if (config == nullptr) {
        config = new Config();

        config->multiplier  = 10;
        config->bios_path   = bios_path;
        config->framebuffer = framebuffer;
    }

    // Create emulator object if not done yet
    if (emulator == nullptr) {
        emulator = new Emulator(config);
    }

    // Apply config and load game.
    emulator->reloadConfig();
    emulator->loadGame(cart);

    // Setup sound output
    setupSound(&emulator->getAPU());

    emu_state = EmulationState::Running;

    // Start FPS counting
    frames = 0;
    timer_fps->start();

    // Start emulating
    timer_run->start();

    screen->grabKeyboard();
}

void MainWindow::pauseClicked() {
    switch (emu_state) {
        case EmulationState::Paused:
            timer_run->start();
            timer_fps->start();

            emu_state = EmulationState::Running;
            break;

        case EmulationState::Running:
            timer_run->stop();
            timer_fps->stop();
            
            emu_state = EmulationState::Paused;
            break;
    }
}

void MainWindow::stopClicked() {
    timer_run->stop();
    timer_fps->stop();

    emu_state = EmulationState::Stopped;
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
