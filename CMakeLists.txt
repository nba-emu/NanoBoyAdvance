cmake_minimum_required (VERSION 2.6)
project(NanoboyAdvance)

# Compiler settings
option(DEBUG "Build for debug purposes" ON)
if (DEBUG)
    add_definitions(-std=c++11 -g -O0 -DDEBUG)
else()
    add_definitions(-std=c++11 -Ofast -funroll-loops)
endif()

# The version number.
#set(VERSION_MAJOR 0)
#set(VERSION_MINOR 1)

# Create header file
#configure_file (
#    "${PROJECT_SOURCE_DIR}/src/version.h.in"
#    "${PROJECT_BINARY_DIR}/version.h"
#    )

# Frontend selection
option(QT_GUI "Build Qt based frontend" ON)
option(SDL2 "Build SDL2 based frontend" ON)
if (NOT QT_GUI AND NOT SDL2)
    message(FATAL_ERROR "No frontend selected. Enable QT_GUI or SDL2.")
endif()

# Add SDL2 if neccessary
if (SDL2)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)
    include (FindSDL2 REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(PNG REQUIRED)
    
    if (SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "SDL2 not found.")
    endif()

    if (PNG_FOUND)
        include_directories(${PNG_INCLUDE_DIRS})
    else()
        # TODO: just built without screenshot support then
        message(FATAL_ERROR "PNG not found.")
    endif()
endif()

# Add Qt if neccessary
if (QT_GUI)
    # TODO: also support Qt4
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    set(QT_DEPS Qt5::Gui Qt5::Widgets)
endif()

add_subdirectory(src)