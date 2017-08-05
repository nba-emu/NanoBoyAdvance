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

#include "mainwindow.hpp"

using namespace GameBoyAdvance;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("NanoboyAdvance");
    
    setupMenu();
}

MainWindow::~MainWindow() {
    
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

void MainWindow::openGame() {
    
}