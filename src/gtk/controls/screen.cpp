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

#include <stdexcept>
#include "screen.hpp"
#include "util/integer.hpp"
#include <iostream>

Screen::Screen() {
  auto gl_config = Gdk::GL::Config::create(Gdk::GL::MODE_RGBA |
                       Gdk::GL::MODE_DOUBLE);
  
  if (!gl_config) {
    throw std::runtime_error("cannot create gl_config");
  }
  
  set_gl_capability(gl_config);
}

Screen::~Screen() {
  glDeleteTextures(1, &m_texture);
}

void Screen::on_realize() {
  Gtk::DrawingArea::on_realize();
  
  auto gl_window = get_gl_window();
  
  if (!gl_window->gl_begin(get_gl_context())) {
    return;
  }
  
  // basic GL setup
  glClearColor(0, 0, 0, 1);
  glViewport(0, 0, get_width(), get_height());
  
  // setup matrices (load identity)
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  // setup texture
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  gl_window->gl_end();
}

bool Screen::on_configure_event(GdkEventConfigure* event) {
  auto gl_window = get_gl_window();

  if (!gl_window->gl_begin(get_gl_context())) {
    return false;
  }
  glViewport(0, 0, get_width(), get_height());
  gl_window->gl_end();
  
  return true;
}

bool Screen::on_expose_event(GdkEventExpose* event) {
  auto gl_window = get_gl_window();

  if (!gl_window->gl_begin(get_gl_context())) {
    return false;
  }
  glClear(GL_COLOR_BUFFER_BIT);
  
  //auto fb = m_framebuffer;
  //
  //if (fb != nullptr) {
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb->get_width(), fb->get_height(), 
  //         0, GL_BGRA, GL_UNSIGNED_BYTE, fb->get_pixels());
  //}
  
  glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex2i(-1, -1);
    
    glTexCoord2i(1, 0);
    glVertex2i(1, -1);

    glTexCoord2i(1, 1);
    glVertex2i(1, 1);

    glTexCoord2i(0, 1);
    glVertex2i(-1, 1);
  glEnd();
  
  gl_window->swap_buffers();
  gl_window->gl_end();
  
  return true;
}