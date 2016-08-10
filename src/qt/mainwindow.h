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
#include <QLabel>
#include <QTimer>

#include "gba/gba.h"
#include "screen.h"

///////////////////////////////////////////////////////////
/// \file    mainwindow.h
/// \author  Frederic Meyer
/// \date    August 8th, 2016
/// \class   MainWindow
/// \brief   Main Window of the Qt frontend.
///
///////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      Constructor
    /// \param   parent            The parent widget.
    ///
    ///////////////////////////////////////////////////////////
    MainWindow(QWidget *parent = 0);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      Destructor
    ///
    ///////////////////////////////////////////////////////////
    ~MainWindow();

public slots:
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \data    August 8th, 2016
    /// \fn      openGame
    /// \brief   Asks the user to open a ROM and runs it.
    ///
    ///////////////////////////////////////////////////////////
    void openGame();

private:

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \data    August 8th, 2016
    /// \fn      runGame
    /// \brief   Loads a specific ROM in the emulator.
    ///
    /// \param  rom_file  ROM path.
    ///
    ///////////////////////////////////////////////////////////
    void runGame(const QString &rom_file);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \data    August 8th, 2016
    /// \fn      keyToGBA
    /// \brief   Converts a Qt key to the GBA::Key enum.
    ///
    ///////////////////////////////////////////////////////////
    NanoboyAdvance::GBA::Key keyToGBA(int key);

    ///////////////////////////////////////////////////////////
    // Class members (Widgets)
    //
    ///////////////////////////////////////////////////////////
    QAction* m_OpenFileAction;
    QAction* m_CloseAction;
    QAction* m_OpenSettingsAction;
    QAction* m_AboutAppAction;
    QAction* m_AboutQtAction;
    QMenu* m_FileMenu;
    QMenu* m_EditMenu;
    QMenu* m_HelpMenu;
    QMenuBar* m_MenuBar;
    QStatusBar* m_StatusBar;
    QLabel* m_StatusMessage;
    Screen* m_Screen;
    QTimer* m_Timer;
    QTimer* m_FPSTimer;

    ///////////////////////////////////////////////////////////
    // Class members (Emulator)
    //
    ///////////////////////////////////////////////////////////
    NanoboyAdvance::GBA* m_GBA {nullptr};
    u16* m_Buffer;
    int m_Frames {0};
};


#endif  // __NBA_MAINWINDOW_H__
