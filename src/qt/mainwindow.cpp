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

using namespace NanoboyAdvance;

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    setWindowTitle("NanoboyAdvance");

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

    // Create status bar
    statusbar = new QStatusBar(this);
    statusbar->showMessage(tr("Idle..."));

    // Window layout
    screen->setBaseSize(480, 320);
    screen->setFixedSize(480, 320);
    screen->setSizeIncrement(1, 1);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMenuBar(menubar);
    layout->addWidget(screen);
    layout->addWidget(statusbar);
    setLayout(layout);
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

    if (!File::Exists(file.toStdString()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Cannot find file ") + QFileInfo(file).baseName());
        box.setWindowTitle(tr("File error"));
        box.exec();
        return;
    }
}

void MainWindow::closeApp()
{
    QApplication::quit();
}