///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>


#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("NanoboyAdvance");

    // Setup menu
    menuBar = new QMenuBar(this);
    fileMenu = menuBar->addMenu(tr("&File"));
    editMenu = menuBar->addMenu(tr("&Edit"));
    helpMenu = menuBar->addMenu(tr("&?"));

    // Setup file menu
    openFileAction = fileMenu->addAction(tr("&Open"));
    closeAction = fileMenu->addAction(tr("&Quit"));
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openGame);
    connect(closeAction, &QAction::triggered, this, &QApplication::quit);

    // Setup edit menu
    openSettingsAction = editMenu->addAction(tr("Settings"));
    openSettingsAction->setMenuRole(QAction::PreferencesRole);
    connect(openSettingsAction, &QAction::triggered, [this] {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText(tr("The settings dialog is not yet implemented."));
        box.setWindowTitle(tr("Error"));
        box.exec();
    });

    // Setup help menu
    aboutNanoboyAdvanceAction = helpMenu->addAction(tr("About NanoboyAdvance"));
    aboutNanoboyAdvanceAction->setMenuRole(QAction::AboutRole);
    connect(aboutNanoboyAdvanceAction, &QAction::triggered, [this] {
        QMessageBox::information(this, tr("About Nanoboy Advance"), tr("A fast, modern GBA emulator."));
    });
    aboutQtAction = helpMenu->addAction(tr("About Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);

    // Create status bar
    statusBar = new QStatusBar(this);
    statusBar->showMessage(tr("Idle..."));
    setStatusBar(statusBar);

    // Setup GL screen
    screen = new Screen(this);
    connect(screen, &Screen::keyPress, this, &MainWindow::keyPress);
    connect(screen, &Screen::keyRelease, this, &MainWindow::keyRelease);
    setCentralWidget(screen);

    // Create emulator timer
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, &MainWindow::timerTick);
}

MainWindow::~MainWindow() {
    delete gba;
}

void MainWindow::runGame(const QString &rom_file) {
    QFileInfo rom_info(rom_file);
    QString save_file = rom_info.path() + QDir::separator() + rom_info.completeBaseName() + ".sav";
    
    delete gba;

    try {
        gba = new NanoboyAdvance::GBA(rom_file.toStdString(), save_file.toStdString(), "bios.bin");
        buffer = gba->GetVideoBuffer();
        timer->start();
        screen->grabKeyboard();
    } catch (const std::runtime_error& e) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr(e.what()));
        box.setWindowTitle(tr("Runtime error"));
        box.exec();
    }
}

NanoboyAdvance::GBA::Key MainWindow::keyToGBA(int key) {
    switch (key) {
        case Qt::Key_A:
            return NanoboyAdvance::GBA::Key::A;
        case Qt::Key_S:
            return NanoboyAdvance::GBA::Key::B;
        case Qt::Key_Backspace:
            return NanoboyAdvance::GBA::Key::Select;
        case Qt::Key_Return:
            return NanoboyAdvance::GBA::Key::Start;
        case Qt::Key_Right:
            return NanoboyAdvance::GBA::Key::Right;
        case Qt::Key_Left:
            return NanoboyAdvance::GBA::Key::Left;
        case Qt::Key_Up:
            return NanoboyAdvance::GBA::Key::Up;
        case Qt::Key_Down:
            return NanoboyAdvance::GBA::Key::Down;
        case Qt::Key_W:
            return NanoboyAdvance::GBA::Key::R;
        case Qt::Key_Q:
            return NanoboyAdvance::GBA::Key::L;
        default:
            return NanoboyAdvance::GBA::Key::None;
    }
}

void MainWindow::openGame() {
    QString file;
    QFileDialog dialog(this);

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("GameBoy Advance ROMs (*.gba *.agb)");

    if (!dialog.exec())
        return;

    file = dialog.selectedFiles().at(0);
    if (!QFile::exists(file)) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Cannot find file ") + QFileInfo(file).fileName());
        box.setWindowTitle(tr("File error"));
        box.exec();
        return;
    }

    runGame(file);
}

void MainWindow::timerTick() {
    gba->Frame();
    if (gba->HasRendered()) {
        screen->updateTexture(buffer, 240, 160);
    }
}

void MainWindow::keyPress(int key) {
    if (key == Qt::Key_Space) {
        gba->SetSpeedUp(10);
    } else {
        gba->SetKeyState(keyToGBA(key), true);
    }
}

void MainWindow::keyRelease(int key) {
    if (key == Qt::Key_Space) {
        gba->SetSpeedUp(1);
    } else {
        gba->SetKeyState(keyToGBA(key), false);
    }
}
