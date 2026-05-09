# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(OpenGL REQUIRED)
find_package(SDL2 2.0.10 REQUIRED COMPONENTS SDL2 OPTIONAL_COMPONENTS SDL2main CONFIG)
find_package(toml11 CONFIG OPTIONAL)

set(SOURCES
  src/device/ogl_video_device.cpp
  src/device/sdl_audio_device.cpp
  src/loader/bios.cpp
  src/loader/rom.cpp
  src/loader/save_state.cpp
  src/writer/save_state.cpp
  src/config.cpp
  src/emulator_thread.cpp
  src/frame_limiter.cpp
  src/game_db.cpp
)

set(HEADERS
  src/device/shader/color_higan.glsl.hpp
  src/device/shader/color_agb.glsl.hpp
  src/device/shader/common.glsl.hpp
  src/device/shader/lcd_ghosting.glsl.hpp
  src/device/shader/lcd1x.glsl.hpp
  src/device/shader/output.glsl.hpp
  src/device/shader/sharp_bilinear.glsl.hpp
  src/device/shader/xbrz.glsl.hpp
)

set(HEADERS_PUBLIC
  include/platform/device/ogl_video_device.hpp
  include/platform/device/sdl_audio_device.hpp
  include/platform/loader/bios.hpp
  include/platform/loader/rom.hpp
  include/platform/loader/save_state.hpp
  include/platform/writer/save_state.hpp
  include/platform/config.hpp
  include/platform/emulator_thread.hpp
  include/platform/frame_limiter.hpp
  include/platform/game_db.hpp
)

add_library(platform-core STATIC)
target_sources(platform-core PRIVATE ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(platform-core PRIVATE src PUBLIC include)
target_link_libraries(platform-core PUBLIC nba toml11::toml11 SDL2::SDL2 OpenGL::GL glad PRIVATE unarr)
