cmake_minimum_required(VERSION 2.8)

# Must be set before project
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(NanoboyAdvance CXX)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

option(FRONTEND_QT "Enable Qt5 frontend" ON)
option(FRONTEND_SDL "Enable SDL2 frontend" ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

  option(PROFILE_GPROF "Profile with gprof" OFF)
  if (PROFILE)
    add_definitions(-pg)
    set(CMAKE_EXE_LINKER_FLAGS -pg)
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

add_subdirectory(source)
