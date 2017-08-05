cmake_minimum_required (VERSION 2.6)

# === BASIC CMAKE SETUP === #

project(NanoboyAdvance)

# Set CMakeTests path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

# The version number.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

# === FRONTENDS === #

# Frontend selection
option(SDL2_GUI "Build SDL2 based GUI" ON)

# SDL2 - required *atleast* for sound
include (FindSDL2 REQUIRED)
find_package(SDL2 REQUIRED)

# libpng - required for screenshots
find_package(PNG  REQUIRED)

# === COMPILATION MODES === #

# Statically link stdlibs?
option(STATIC_STDLIB "Link libgcc and libg++ statically" OFF)
if (WIN32)
    set(STATIC_STDLIB ON)
endif()
if (STATIC_STDLIB)
    add_definitions(-static-libgcc -static-libstdc++)
endif()

# Debug Mode?
option(DEBUG "Build for debugging" OFF)
if (DEBUG)
    add_definitions(-std=c++11 -g -O0 -fpermissive -DDEBUG)
else()
    add_definitions(-std=c++11 -Ofast -fpermissive -finline-functions)
endif()

# Perform profiling?
option(PROFILE "Collect GPROF sample data" OFF)
if (PROFILE)
    add_definitions(-pg)
    set(CMAKE_EXE_LINKER_FLAGS -pg)
endif()

add_subdirectory(src)
