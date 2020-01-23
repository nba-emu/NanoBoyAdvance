cmake_minimum_required(VERSION 2.8)
project(NanoboyAdvance C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(PLATFORM "sdl" CACHE STRING "Build Target (sdl, ...)")

set(COMPILE_FLAGS   "${COMPILE_FLAGS} -Ofast")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS -pg)

add_subdirectory(source)
