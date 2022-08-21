cmake_minimum_required(VERSION 3.2)
project(NanoBoyAdvance)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

if (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
  set(SWITCH TRUE)
  add_compile_definitions(SWITCH)
else()
  set(SWITCH FALSE)
endif()

add_subdirectory(external ${CMAKE_CURRENT_BINARY_DIR}/external)
add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (SWITCH)
  add_subdirectory(src/platform/nx ${CMAKE_CURRENT_BINARY_DIR}/bin/nx)
else()
  option(PLATFORM_QT "Build Qt frontend" ON)

  if (PLATFORM_QT)
    add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
  endif()
endif()