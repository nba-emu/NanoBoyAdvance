project(NanoBoyAdvance-Qt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  main.cpp
  screen.cpp
  main_window.cpp
)

set(HEADERS
  screen.hpp
  main_window.hpp
)

# CHECKME: dunno, is this required?
set(CMAKE_AUTOMOC ON)

# CHECKME: possibly we can get rid of all OpenGL dependencies here.
find_package(OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(QT_DEPS Qt5::Gui Qt5::Widgets Qt5::OpenGL)

add_executable(NanoBoyAdvance-Qt ${SOURCES} ${HEADERS})
target_link_libraries(NanoBoyAdvance-Qt platform-core ${QT_DEPS} ${OPENGL_gl_LIBRARY})
set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")