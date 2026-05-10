# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.24)

include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<CONFIG>")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IS_RELEASE ON)
else()
  set(IS_RELEASE OFF)
endif()

project(NanoBoyAdvance LANGUAGES C CXX)

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
endif()

option(PLATFORM_QT "Build the Qt 6 frontend." ON)

add_subdirectory(thirdparty)

add_subdirectory(src/nba)
add_subdirectory(src/platform/core)

if (PLATFORM_QT)
  add_subdirectory(src/platform/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()
