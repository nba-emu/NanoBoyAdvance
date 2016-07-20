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

#include "gba/gba.h"
#include "screen.h"
#include <string>
#include <QWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>

#pragma once

class MainWindow : public QWidget
{
    Q_OBJECT

    QAction* open_file;
    QAction* close_app;
    QMenu* file_menu;
    QMenu* help_menu;
    QMenuBar* menubar;

    QStatusBar* statusbar;
    Screen* screen;

    QTimer* timer;
    NanoboyAdvance::GBA* gba {nullptr};

    void runGame(std::string rom_file);
    NanoboyAdvance::GBA::Key keyToGBA(int key);
public slots:
    void openGame();
    void closeApp();
    void timerTick();
    void keyPress(int key);
    void keyRelease(int key);
public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();
};