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


#ifndef __NBA_SCREEN_H__
#define __NBA_SCREEN_H__


#include <QGLWidget>


class Screen : public QGLWidget
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
    Screen(QWidget *parent = 0);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      Destructor
    ///
    ///////////////////////////////////////////////////////////
    ~Screen();

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      updateTexture
    /// \brief   Updates the texture/image that is drawn.
    ///
    /// \param  pixels  ARG888 pixel data.
    /// \param  width   Texture width
    /// \param  height  Texture height
    ///
    ///////////////////////////////////////////////////////////
    void updateTexture(unsigned int *pixels, int width, int height);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      sizeHint
    /// \brief   A hint of what the optimal size should be.
    ///
    ///////////////////////////////////////////////////////////
    QSize sizeHint() const;

signals:
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      keyPress
    /// \brief   Indicates that a key was being pressed.
    ///
    /// \param  key Key Code
    ///
    ///////////////////////////////////////////////////////////
    void keyPress(int key);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      keyRelease
    /// \brief   Indicates that a key was being released.
    ///
    /// \param  key Key Code
    ///
    ///////////////////////////////////////////////////////////
    void keyRelease(int key);
    
protected:
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      initializeGL
    /// \brief   Setups the OpenGL environment.
    ///
    ///////////////////////////////////////////////////////////
    void initializeGL();

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      paintGL
    /// \brief   Updates the screen.
    ///
    ///////////////////////////////////////////////////////////
    void paintGL();

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      resizeGL
    /// \brief   Handles screen resizing.
    ///
    ///////////////////////////////////////////////////////////
    void resizeGL(int width, int height);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      keyPressEvent
    /// \brief   keyPress event, throws keyPress signal
    ///
    /// \param  event Event Information
    ///
    ///////////////////////////////////////////////////////////
    void keyPressEvent(QKeyEvent *event);

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    August 8th, 2016
    /// \fn      keyReleaseEvent
    /// \brief   keyRelease event, throws keyRelease signal
    ///
    /// \param  event Event Information
    ///
    ///////////////////////////////////////////////////////////
    void keyReleaseEvent(QKeyEvent *event);

private:
    ///////////////////////////////////////////////////////////
    // Class members
    //
    ///////////////////////////////////////////////////////////
    GLuint texture;
};


#endif  // __NBA_SCREEN_H__
