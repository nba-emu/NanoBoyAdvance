# SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
# SPDX-License-Identifier: 0BSD

cmake_minimum_required(VERSION 3.2...4.0 FATAL_ERROR)

project(atom CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Which atom modules are included from this CMakeLists:
option(ATOM_INCLUDE_LOGGER "Enable the atom-logger module" ON)
option(ATOM_INCLUDE_MATH "Enable the atom-math module" ON)

if(NOT TARGET fmt::fmt)
  find_package(fmt REQUIRED)
endif()

add_subdirectory(common)

if(ATOM_INCLUDE_LOGGER)
  add_subdirectory(logger)
endif()