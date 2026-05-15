# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED COMPONENTS SDL3)

set(SOURCES
  src/device/ogl_video_device.cc
  src/device/sdl_audio_device.cc
  src/loader/bios.cc
  src/loader/rom.cc
  src/loader/save_state.cc
  src/writer/save_state.cc
  src/config.cc
  src/emulator_thread.cc
  src/frame_limiter.cc
  src/game_db.cc
)

set(HEADERS
  src/device/shader/color_higan.glsl.hh
  src/device/shader/color_agb.glsl.hh
  src/device/shader/common.glsl.hh
  src/device/shader/lcd_ghosting.glsl.hh
  src/device/shader/lcd1x.glsl.hh
  src/device/shader/output.glsl.hh
  src/device/shader/sharp_bilinear.glsl.hh
  src/device/shader/xbrz.glsl.hh
)

set(HEADERS_PUBLIC
  include/platform/device/ogl_video_device.hh
  include/platform/device/sdl_audio_device.hh
  include/platform/loader/bios.hh
  include/platform/loader/rom.hh
  include/platform/loader/save_state.hh
  include/platform/writer/save_state.hh
  include/platform/config.hh
  include/platform/emulator_thread.hh
  include/platform/frame_limiter.hh
  include/platform/game_db.hh
)

add_library(platform-core STATIC)
target_sources(platform-core PRIVATE ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(platform-core PRIVATE src PUBLIC include)
target_link_libraries(platform-core PUBLIC nba toml11::toml11 SDL3::SDL3 OpenGL::GL glad PRIVATE unarr)
