cmake_minimum_required(VERSION 2.8)
project(NanoboyAdvance C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

set(PLATFORM "sdl" CACHE STRING "Build Target (sdl, ...)")

set(COMPILE_FLAGS   "${COMPILE_FLAGS} -Ofast")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} -std=c++1z")

add_subdirectory(src)
