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

#include "screen.hpp"
#include <QtWidgets>

Screen::Screen(QWidget* parent) : QGLWidget(parent)
{ }

Screen::~Screen() {
    glDeleteTextures(1, &m_texture);
}

void Screen::updateTexture(u32* pixels, int width, int height) {
    // Update texture pixels
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA, 
        width, 
        height, 
        0, 
        GL_BGRA, 
        GL_UNSIGNED_BYTE, 
        pixels
    );
    
    // Redraw screen
    updateGL();
}

auto Screen::sizeHint() const -> QSize {
    // Default size, 2 times of GBA screen.
    return QSize {480, 320};
}

void Screen::initializeGL() {
    // Clear screen, black color
    qglClearColor(Qt::black);
    
    // Enable textures
    glEnable(GL_TEXTURE_2D);

    // Generate one texture to store screen pixels
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    // Set texture interpolation mode to nearest
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Screen::paintGL() {
    float w = static_cast<float>(width());
    float h = static_cast<float>(height());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glBegin(GL_QUADS); 
    {
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);
        
        glTexCoord2f(1.0f, 0);
        glVertex2f(w, 0);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(w, h);

        glTexCoord2f(0, 1.0f);
        glVertex2f(0, h);
    }
    glEnd();
}

void Screen::resizeGL(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_MODELVIEW);
}

void Screen::keyPressEvent(QKeyEvent* event) {
    emit keyPress(event->key());
}

void Screen::keyReleaseEvent(QKeyEvent* event) {
    emit keyRelease(event->key());
}