/*
* Copyright (C) 2016 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "util/file.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

using namespace std;
using namespace NanoboyAdvance;

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    setWindowTitle("NanoboyAdvance");

    // TODO: spawn window at another location, maybe in the middle or depending of the cursor location?
    this->setGeometry(0, 0, 480, 320);

    // Setup menu
    menubar = new QMenuBar(this);
    file_menu = menubar->addMenu(tr("&File"));
    help_menu = menubar->addMenu(tr("&?"));

    // Setup file menu
    open_file = file_menu->addAction(tr("&Open"));
    close_app = file_menu->addAction(tr("&Close"));
    connect(open_file, SIGNAL(triggered()), this, SLOT(openGame()));
    connect(close_app, SIGNAL(triggered()), this, SLOT(closeApp()));

    // Setup GL screen
    screen = new Screen(this);
    screen->resize(480, 320);
    connect(screen, SIGNAL(keyPress(int)), this, SLOT(keyPress(int)));
    connect(screen, SIGNAL(keyRelease(int)), this, SLOT(keyRelease(int)));

    // Create status bar
    statusbar = new QStatusBar(this);
    statusbar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    statusbar->showMessage(tr("Idle..."));

    // Window layout
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMenuBar(menubar);
    layout->addWidget(screen);
    layout->addWidget(statusbar);
    setLayout(layout);

    // Create emulator timer
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(16);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
}

MainWindow::~MainWindow()
{
    if (gba != nullptr)
        delete gba;
}

void MainWindow::runGame(QString rom_file)
{
    QString save_file = QFileInfo(rom_file).baseName() + ".sav";

    if (gba != nullptr)
        delete gba;

    try
    {
        gba = new GBA(rom_file.toStdString(), save_file.toStdString(), "bios.bin");
        timer->start();
        screen->grabKeyboard();
    }
    catch (runtime_error& e)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr(e.what()));
        box.setWindowTitle(tr("Runtime error"));
        box.exec();
    }
}

GBA::Key MainWindow::keyToGBA(int key)
{
    switch (key)
    {
    case Qt::Key_A: return GBA::Key::A;
    case Qt::Key_S: return GBA::Key::B;
    case Qt::Key_Backspace: return GBA::Key::Select;
    case Qt::Key_Return: return GBA::Key::Start;
    case Qt::Key_Right: return GBA::Key::Right;
    case Qt::Key_Left: return GBA::Key::Left;
    case Qt::Key_Up: return GBA::Key::Up;
    case Qt::Key_Down: return GBA::Key::Down;
    case Qt::Key_W: return GBA::Key::R;
    case Qt::Key_Q: return GBA::Key::L;
    }

    return GBA::Key::None;
}

void MainWindow::openGame()
{
    QString file;
    QFileDialog dialog(this);

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("GameBoy Advance ROMs (*.gba *.agb)");

    if (!dialog.exec())
        return;

    file = dialog.selectedFiles().at(0);
    if (!QFile::exists(file))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Cannot find file ") + QFileInfo(file).baseName());
        box.setWindowTitle(tr("File error"));
        box.exec();
        return;
    }

    runGame(file);
}

void MainWindow::closeApp()
{
    QApplication::quit();
}

void MainWindow::timerTick()
{
    u32* frame = gba->Frame();
    screen->updateTexture(frame, 240, 160);
    free(frame);
}

void MainWindow::keyPress(int key)
{
    gba->SetKeyState(keyToGBA(key), true);
}

void MainWindow::keyRelease(int key)
{
    gba->SetKeyState(keyToGBA(key), false);
}
