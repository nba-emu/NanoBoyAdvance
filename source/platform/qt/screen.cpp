/**
  * Copyright (C) 2020 fleroviux (Frederic Meyer)
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

void Screen::Draw(std::uint32_t* buffer) {
  emit SignalDraw(buffer);
}

void Screen::DrawSlot(std::uint32_t* buffer) {
  glBindTexture(GL_TEXTURE_2D, texture);

  /* Update texture pixels */
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    240, /* TODO: do not hardcode the dimensions? */
    160,
    0,
    GL_BGRA,
    GL_UNSIGNED_BYTE,
    buffer
  );

  /* Redraw screen */
  update();
}

void Screen::initializeGL() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Screen::paintGL() {
  glViewport(viewport_x, 0, viewport_width, viewport_height);
  glBindTexture(GL_TEXTURE_2D, texture);

  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);
    glVertex2f(-1.0f, 1.0f);

    glTexCoord2f(1.0f, 0);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(0, 1.0f);
    glVertex2f(-1.0f, -1.0f);
  }
  glEnd();
}

void Screen::resizeGL(int width, int height) {
  viewport_width = height + height / 2;
  viewport_height = height;
  viewport_x = (width - viewport_width) / 2;
}