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


#ifndef __NBA_MAINWINDOW_H__
#define __NBA_MAINWINDOW_H__


#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>

#include "gba/gba.h"
#include "screen.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openGame();
    void timerTick();
    void keyPress(int key);
    void keyRelease(int key);

private:
    QAction *openFileAction;
    QAction *closeAction;
    QAction *openSettingsAction;
    QAction *aboutNanoboyAdvanceAction;
    QAction *aboutQtAction;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    QMenuBar *menuBar;

    QStatusBar *statusBar;
    Screen *screen;

    QTimer *timer;
    NanoboyAdvance::GBA *gba;
    u32 *buffer;

    void runGame(const QString &rom_file);
    NanoboyAdvance::GBA::Key keyToGBA(int key);
};


#endif  // __NBA_MAINWINDOW_H__
