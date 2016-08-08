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

using namespace NanoboyAdvance;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("NanoboyAdvance");

    // Setup menu
    m_MenuBar = new QMenuBar {this};
    m_FileMenu = m_MenuBar->addMenu(tr("&File"));
    m_EditMenu = m_MenuBar->addMenu(tr("&Edit"));
    m_HelpMenu = m_MenuBar->addMenu(tr("&?"));
    setMenuBar(m_MenuBar);

    // Setup file menu
    m_OpenFileAction = m_FileMenu->addAction(tr("&Open"));
    m_CloseAction = m_FileMenu->addAction(tr("&Quit"));
    connect(m_OpenFileAction, &QAction::triggered, this, &MainWindow::openGame);
    connect(m_CloseAction, &QAction::triggered, this, &QApplication::quit);

    // Setup edit menu
    m_OpenSettingsAction = m_EditMenu->addAction(tr("Settings"));
    m_OpenSettingsAction->setMenuRole(QAction::PreferencesRole);
    connect(m_OpenSettingsAction, &QAction::triggered,
        [this]
        {
            QMessageBox box {this};
            box.setIcon(QMessageBox::Information);
            box.setText(tr("The settings dialog is not yet implemented."));
            box.setWindowTitle(tr("Error"));
            box.exec();
        });

    // Setup help menu
    m_AboutAppAction = m_HelpMenu->addAction(tr("About NanoboyAdvance"));
    m_AboutAppAction->setMenuRole(QAction::AboutRole);
    connect(m_AboutAppAction, &QAction::triggered,
        [this]
        {
            QMessageBox::information(this, tr("About NanoboyAdvance"), tr("A fast, modern GBA emulator."));
        });
    m_AboutQtAction = m_HelpMenu->addAction(tr("About Qt"));
    m_AboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(m_AboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);

    // Create status bar
    m_StatusMessage = new QLabel(this);
    m_StatusBar = new QStatusBar(this);
    m_StatusMessage->setText(tr("Idle..."));
    m_StatusBar->addPermanentWidget(m_StatusMessage);
    setStatusBar(m_StatusBar);

    // Setup GL screen
    m_Screen = new Screen {this};
    connect(m_Screen, &Screen::keyPress, this, &MainWindow::keyPress);
    connect(m_Screen, &Screen::keyRelease, this, &MainWindow::keyRelease);
    setCentralWidget(m_Screen);

    // Create emulator timer
    m_Timer = new QTimer {this};
    m_Timer->setSingleShot(false);
    m_Timer->setInterval(17);
    connect(m_Timer, &QTimer::timeout, this, &MainWindow::timerTick);

    // Create FPS counting timer
    m_FPSTimer = new QTimer {this};
    m_FPSTimer->setSingleShot(false);
    m_FPSTimer->setInterval(1000);
    connect(m_FPSTimer, &QTimer::timeout, this, &MainWindow::fpsTimerTick);
}

MainWindow::~MainWindow()
{
    delete m_GBA;
}

void MainWindow::runGame(const QString &rom_file)
{
    QFileInfo rom_info {rom_file};
    QString save_file = rom_info.path() + QDir::separator() + rom_info.completeBaseName() + ".sav";

    try
    {
        delete m_GBA;
        m_GBA = new GBA {rom_file.toStdString(), save_file.toStdString(), "bios.bin"};
        m_Buffer = m_GBA->GetVideoBuffer();
        m_Timer->start();
        m_FPSTimer->start();
        m_Screen->grabKeyboard();
    }
    catch (const std::runtime_error& e)
    {
        QMessageBox box {this};
        box.setIcon(QMessageBox::Critical);
        box.setText(tr(e.what()));
        box.setWindowTitle(tr("Runtime error"));
        box.exec();
    }
}

GBA::Key MainWindow::keyToGBA(int key) {
    switch (key)
    {
        case Qt::Key_A:
            return GBA::Key::A;
        case Qt::Key_S:
            return GBA::Key::B;
        case Qt::Key_Backspace:
            return GBA::Key::Select;
        case Qt::Key_Return:
            return GBA::Key::Start;
        case Qt::Key_Right:
            return GBA::Key::Right;
        case Qt::Key_Left:
            return GBA::Key::Left;
        case Qt::Key_Up:
            return GBA::Key::Up;
        case Qt::Key_Down:
            return GBA::Key::Down;
        case Qt::Key_W:
            return GBA::Key::R;
        case Qt::Key_Q:
            return GBA::Key::L;
        default:
            return GBA::Key::None;
    }
}

void MainWindow::openGame()
{
    QFileDialog dialog {this};

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("GameBoy Advance ROMs (*.gba *.agb)");

    if (!dialog.exec())
        return;

    QString file = dialog.selectedFiles().at(0);
    if (!QFile::exists(file))
    {
        QMessageBox box {this};
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Cannot find file ") + QFileInfo(file).fileName());
        box.setWindowTitle(tr("File error"));
        box.exec();
        return;
    }

    runGame(file);
}

void MainWindow::timerTick()
{
    m_GBA->Frame();
    if (m_GBA->HasRendered())
    {
        m_Screen->updateTexture(m_Buffer, 240, 160);
        m_Frames++;
    }
}

void MainWindow::fpsTimerTick()
{
    std::string message = std::to_string(m_Frames) + " FPS";
    m_StatusMessage->setText(QString(message.c_str()));
    m_Frames = 0;
}

void MainWindow::keyPress(int key)
{
    if (key == Qt::Key_Space)
        m_GBA->SetSpeedUp(10);
    else
        m_GBA->SetKeyState(keyToGBA(key), true);
}

void MainWindow::keyRelease(int key)
{
    if (key == Qt::Key_Space)
        m_GBA->SetSpeedUp(1);
    else
        m_GBA->SetKeyState(keyToGBA(key), false);
}
