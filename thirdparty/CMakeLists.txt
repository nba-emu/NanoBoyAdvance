# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Glad 2.0.0 GLOBAL QUIET)
if(NOT WEIRD_STUPID_CI_BULLSHIT_DISABLE_TOML11_PRETTY_PLEASE)
  find_package(toml11 4.4.0 GLOBAL QUIET)
endif()

find_package(unarr 1.1.0 GLOBAL QUIET)

if(Glad_FOUND)
  glad_add_library(glad_gl_core_33 STATIC LANGUAGE c REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE)
  add_library(glad ALIAS glad_gl_core_33)
  alias()
else()
  add_library(glad STATIC
    glad-2.0.8/include/glad/gl.h
    glad-2.0.8/include/KHR/khrplatform.h

    glad-2.0.8/src/gl.c
  )

  target_include_directories(glad PUBLIC glad-2.0.8/include)
endif()

if(NOT toml11_FOUND)
  add_subdirectory(toml11-4.4.0-patch)
endif()

if(NOT unarr_FOUND)
  add_subdirectory(unarr-1.1.1-patch)
endif()
