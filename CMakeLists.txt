# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<CONFIG>")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IS_RELEASE OFF)
else()
  set(IS_RELEASE OFF)
endif()

if(NOT DEFINED Python_ROOT_DIR AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/.venv)
  set(Python_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/.venv)
endif()

project(NanoBoyAdvance LANGUAGES C CXX)

option(PLATFORM_QT "Build the Qt 6 frontend." ON)

add_subdirectory(thirdparty)

add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (PLATFORM_QT)
  add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()
