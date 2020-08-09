set(SOURCES
  main.cpp
  screen.cpp
  mainwindow.cpp)

set(HEADERS
  screen.hpp
  mainwindow.hpp)

# CHECKME: dunno, is this required?
set(CMAKE_AUTOMOC ON)

# CHECKME: possibly we can get rid of all OpenGL dependencies here.
find_package(OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(QT_DEPS Qt5::Gui Qt5::Widgets Qt5::OpenGL)

include(FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(NanoboyAdvance-Qt ${SOURCES} ${HEADERS})
target_link_libraries(NanoboyAdvance-Qt nba ${QT_DEPS} ${OPENGL_gl_LIBRARY} ${SDL2_LIBRARY})
