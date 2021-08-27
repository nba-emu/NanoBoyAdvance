cmake_minimum_required(VERSION 3.2)
project(NanoBoyAdvance)

option(PLATFORM_SDL2 "Build SDL2 frontend" ON)

add_subdirectory(src/nba)

if (PLATFORM_SDL2)
  add_subdirectory(src/platform/sdl ${CMAKE_CURRENT_BINARY_DIR}/bin/sdl/)
endif()