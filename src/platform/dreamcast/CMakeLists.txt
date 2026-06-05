# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

set(SOURCES
  src/device/dc_video_device.cc
  src/device/dc_audio_device.cc
  src/device/dc_input.cc
  src/dc_config.cc
  src/dc_ui.cc
  src/dc_rom_browser.cc
  src/dc_frontend.cc
  src/main.cc
)

set(HEADERS
  src/device/dc_video_device.hh
  src/device/dc_audio_device.hh
  src/device/dc_input.hh
  src/dc_config.hh
  src/dc_paths.hh
  src/dc_ui.hh
  src/dc_rom_browser.hh
  src/dc_frontend.hh
)

add_executable(NanoBoyAdvance-DC)

target_sources(NanoBoyAdvance-DC PRIVATE ${SOURCES} ${HEADERS})
target_include_directories(NanoBoyAdvance-DC PRIVATE src)
target_link_libraries(NanoBoyAdvance-DC PRIVATE platform-core)

set_target_properties(NanoBoyAdvance-DC PROPERTIES OUTPUT_NAME "NanoBoyAdvance")

install(TARGETS NanoBoyAdvance-DC DESTINATION bin)
