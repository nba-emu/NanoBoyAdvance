set(SOURCES
  main.cpp
  mainwindow.cpp
  screen.cpp
)

set(HEADERS
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


include(FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

if (WIN32)
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/res/app.rc"
        )
    list(APPEND RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/res/app.rc")
    if (MINGW)
        set(CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
        set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
    endif()
endif()

add_executable(NanoboyAdvance ${SOURCES} ${HEADERS} ${RESOURCES})
target_link_libraries(NanoboyAdvance nba ${QT_DEPS} ${OPENGL_gl_LIBRARY} ${SDL2_LIBRARY})
