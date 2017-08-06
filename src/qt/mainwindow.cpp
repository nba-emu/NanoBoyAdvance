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
    setupEmuTimer();
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
    
    setCentralWidget(m_screen);
}

void MainWindow::setupEmuTimer() {
    m_timer = new QTimer {this};
    
    m_timer->setSingleShot(false);
    m_timer->setInterval(17);
    
    connect(m_timer, &QTimer::timeout, this, &MainWindow::nextFrame);
}

void MainWindow::nextFrame() {
    m_emu->run_frame();
    m_screen->updateTexture(m_framebuffer, 240, 160);
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
    auto config = new Config(); // why dynamic
    auto cart   = Cartridge::from_file(rom_file.toStdString());
    
    if (m_emu != nullptr) {
        closeAudio();
        delete m_emu;
    }
    
    // TODO: try reusing an existing instance
    m_emu = new Emulator(config);
    
    config->bios_path   = "bios.bin";
    config->framebuffer = m_framebuffer;
    
    m_emu->load_config();
    m_emu->load_game(cart);
    
    setupSound(&m_emu->get_apu());
    m_timer->start();
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