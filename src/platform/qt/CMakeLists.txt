project(NanoBoyAdvance-Qt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  config.cpp
  keymap_window.cpp
  main.cpp
  screen.cpp
  main_window.cpp
)

set(HEADERS
  config.hpp
  keymap_window.hpp
  screen.hpp
  main_window.hpp
)

set(RESOURCES
)

# CHECKME: dunno, is this required?
set(CMAKE_AUTOMOC ON)

# CHECKME: possibly we can get rid of all OpenGL dependencies here.
find_package(OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(QT_DEPS Qt5::Gui Qt5::Widgets Qt5::OpenGL)

# Append application resources (icon and manifest) on Win32.
if(WIN32)
  list(APPEND RESOURCES rc/app.rc)

  if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
  endif()  
endif()

add_executable(NanoBoyAdvance-Qt ${SOURCES} ${HEADERS} ${RESOURCES})
target_link_libraries(NanoBoyAdvance-Qt platform-core ${QT_DEPS} ${OPENGL_gl_LIBRARY})
set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")