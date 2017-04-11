cmake_minimum_required (VERSION 2.6)
project(NanoboyAdvance)

# TODO: refactor this entire MESS

# Statically link stdlibs?
option(STATIC_STDLIB "Link libgcc and libg++ statically" OFF)
if (WIN32)
    set(STATIC_STDLIB ON)
endif()
if (STATIC_STDLIB)
    add_definitions(-static-libgcc -static-libstdc++)
endif()

# Debug Mode?
option(DEBUG "Build for debug purposes" OFF)
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

# The version number.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

# Frontend selection
option(GTK_GUI  "Build GTKMM GUI"      ON)
option(SDL2_GUI "Build SDL2 based GUI" ON)

# GTKMM extra love
if (GTK_GUI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM gtkmm-3.0)
endif()

# SDL2 related code
if (SDL2_GUI)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)
    
    include (FindSDL2 REQUIRED)
    find_package(SDL2 REQUIRED)

    if (SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "SDL2 not found.")
    endif()
endif()

# libpng - required for screenshots
find_package(PNG  REQUIRED)

if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
else()
    # TODO: just built without screenshot support then
    message(FATAL_ERROR "PNG not found.")
endif()

add_subdirectory(src)
