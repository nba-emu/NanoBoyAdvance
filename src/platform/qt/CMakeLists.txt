project(NanoBoyAdvance-Qt CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/widget/input_window.cpp
  src/widget/main_window.cpp
  src/widget/screen.cpp
  src/config.cpp
  src/main.cpp
)

set(HEADERS
  src/widget/input_window.hpp
  src/widget/main_window.hpp
  src/widget/screen.hpp
  src/config.hpp
)

set(RESOURCES
)

set(CMAKE_AUTOMOC ON)

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
target_include_directories(NanoBoyAdvance-Qt PRIVATE src)
set_target_properties(NanoBoyAdvance-Qt PROPERTIES OUTPUT_NAME "NanoBoyAdvance")