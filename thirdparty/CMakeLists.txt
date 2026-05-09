# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Glad QUIET OPTIONAL)
find_package(unarr QUIET OPTIONAL)

if(Glad_FOUND)
  glad_add_library(glad_gl_core_33 STATIC LANGUAGE c REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE)
  add_library(glad ALIAS glad_gl_core_33)
  alias()
else()
  add_library(glad
    glad-2.0.8/include/glad/gl.h
    glad-2.0.8/include/KHR/khrplatform.h

    glad-2.0.8/src/gl.c
  )

  target_include_directories(glad PUBLIC glad-2.0.8/include)
endif()

if(NOT unarr_FOUND)
  add_subdirectory(unarr-1.1.1-patch)
endif()
