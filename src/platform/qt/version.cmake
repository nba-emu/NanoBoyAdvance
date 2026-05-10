# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Git)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 8)
set(VERSION_PATCH 2)

if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} OUTPUT_VARIABLE VERSION_GIT_BRANCH)
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --match=nevermatch --always --dirty WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} OUTPUT_VARIABLE VERSION_GIT_HASH)

  string(STRIP "${VERSION_GIT_BRANCH}" VERSION_GIT_BRANCH)
  string(STRIP "${VERSION_GIT_HASH}" VERSION_GIT_HASH)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)
