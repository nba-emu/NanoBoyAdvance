cmake_minimum_required(VERSION 3.2)
project(NanoBoyAdvance)

option(PLATFORM_SDL2 "Build SDL2 frontend" ON)
option(PLATFORM_QT "Build Qt frontend" ON)

add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (PLATFORM_SDL2)
  add_subdirectory(src/platform/sdl ${CMAKE_CURRENT_BINARY_DIR}/bin/sdl/)
endif()

if (PLATFORM_QT)
  add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()