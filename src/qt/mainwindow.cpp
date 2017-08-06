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

using namespace GameBoyAdvance;

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
    m_menu_bar = new QMenuBar {this};

    m_file_menu = m_menu_bar->addMenu(tr("&File"));
    m_edit_menu = m_menu_bar->addMenu(tr("&Edit"));
    m_help_menu = m_menu_bar->addMenu(tr("&?"));

    setMenuBar(m_menu_bar);

    setupFileMenu();
}

void MainWindow::setupFileMenu() {
    m_open_file = m_file_menu->addAction(tr("&Open"));
    m_close     = m_file_menu->addAction(tr("&Close"));

    connect(m_open_file, &QAction::triggered, this, &MainWindow::openGame);
    connect(m_close,     &QAction::triggered, this, &QApplication::quit  );
}

void MainWindow::setupScreen() {
    m_screen = new Screen {this};
    
    connect(m_screen, &Screen::keyPress, this,
        [this] (int key) {
            if (m_emu == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                m_config->fast_forward = true;
                return;
            }
            m_emu->set_keystate(MainWindow::qtKeyToEmu(key), true);
        }
    );
    connect(m_screen, &Screen::keyRelease, this,
        [this] (int key) {
            if (m_emu == nullptr) {
                return;
            }
            if (key == Qt::Key_Space) {
                m_config->fast_forward = false;
                return;
            }
            m_emu->set_keystate(MainWindow::qtKeyToEmu(key), false);
        }
    );
    
    setCentralWidget(m_screen);
}

void MainWindow::setupEmuTimers() {
    m_timer     = new QTimer {this};
    m_fps_timer = new QTimer {this};
    
    m_timer->setSingleShot(false);
    m_timer->setInterval(16);
    
    m_fps_timer->setSingleShot(false);
    m_fps_timer->setInterval(1000);
    
    connect(m_timer, &QTimer::timeout, this, &MainWindow::nextFrame);
    connect(m_fps_timer, &QTimer::timeout, this,
        [this] {
            std::string message = std::to_string(m_frames) + " FPS";
            
            m_status_msg->setText(QString(message.c_str()));
            
            m_frames = 0;
        }
    );
}

void MainWindow::setupStatusBar() {
    m_status_msg = new QLabel {this};
    m_status_bar = new QStatusBar {this};
    
    m_status_msg->setText(tr("Idle..."));
    m_status_bar->addPermanentWidget(m_status_msg);
    
    setStatusBar(m_status_bar);
}

void MainWindow::nextFrame() {
    m_emu->run_frame();
    
    m_screen->updateTexture(m_framebuffer, 240, 160);
    
    m_frames++;
}

void MainWindow::openGame() {
    QFileDialog dialog {this};
    
    dialog.setAcceptMode (QFileDialog::AcceptOpen);
    dialog.setFileMode   (QFileDialog::AnyFile   );
    dialog.setNameFilter ("GameBoyAdvance ROMs (*.gba *.agb)");

    if (!dialog.exec()) {
        return;
    }
    
    QString file = dialog.selectedFiles().at(0);
    
    if (!QFile::exists(file)) {
        QMessageBox box {this};
        
        auto dialog_text = tr("Cannot find file ") + 
                           QFileInfo(file).fileName();
        
        box.setIcon(QMessageBox::Critical);
        box.setText(dialog_text);
        box.setWindowTitle(tr("File not found"));
        
        box.exec();
        
        return;
    }

    runGame(file);
}

void MainWindow::runGame(const QString& rom_file) {
    auto cart   = Cartridge::from_file(rom_file.toStdString());
    
    if (m_emu != nullptr) {
        closeAudio();
        delete m_emu;
    }
    if (m_config != nullptr) {
        delete m_config;
    }
    
    m_config = new Config();
    
    // TODO: try reusing an existing instance
    m_emu = new Emulator(m_config);
    
    m_config->multiplier  = 10;
    m_config->bios_path   = "bios.bin";
    m_config->framebuffer = m_framebuffer;
    
    m_emu->load_config();
    m_emu->load_game(cart);
    
    setupSound(&m_emu->get_apu());
    
    m_frames = 0;
    
    m_timer->start();
    m_fps_timer->start();
    
    m_screen->grabKeyboard();
}

void MainWindow::soundCallback(APU* apu, u16* stream, int length) {
    apu->fill_buffer(stream, length);
}

void MainWindow::setupSound(APU* apu) {
    SDL_AudioSpec spec;

    spec.freq     = 44100;
    spec.samples  = 1024;
    spec.format   = AUDIO_U16;
    spec.channels = 2;
    spec.callback = &MainWindow::soundCallback;
    spec.userdata = apu;

    SDL_Init(SDL_INIT_AUDIO);
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void MainWindow::closeAudio() {
    SDL_CloseAudioDevice(1);
}

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