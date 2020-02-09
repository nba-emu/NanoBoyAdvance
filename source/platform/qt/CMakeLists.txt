SET(SOURCES
  main.cpp
  mainwindow.cpp
  screen.cpp
)

SET(HEADERS
  mainwindow.hpp
  screen.hpp
)

# TODO: do we really need this line?
set(CMAKE_AUTOMOC ON)

find_package(OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(QT_DEPS Qt5::Gui Qt5::Widgets Qt5::OpenGL)

ADD_EXECUTABLE(NanoboyAdvance ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(NanoboyAdvance gba ${QT_DEPS} ${OPENGL_gl_LIBRARY})
