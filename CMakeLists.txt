cmake_minimum_required(VERSION 3.2)
project(NanoBoyAdvance)

option(PLATFORM_QT "Build Qt frontend" ON)

add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (PLATFORM_QT)
  add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()