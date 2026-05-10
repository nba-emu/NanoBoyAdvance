# SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
# SPDX-License-Identifier: GPL-3.0-or-later

set(SOURCES
  src/arm/tablegen/tablegen.cc
  src/arm/serialization.cc
  src/bus/bus.cc
  src/bus/io.cc
  src/bus/serialization.cc
  src/bus/timing.cc
  src/hw/apu/channel/noise_channel.cc
  src/hw/apu/channel/quad_channel.cc
  src/hw/apu/channel/wave_channel.cc
  src/hw/apu/hle/mp2k.cc
  src/hw/apu/apu.cc
  src/hw/apu/callback.cc
  src/hw/apu/registers.cc
  src/hw/apu/serialization.cc
  src/hw/ppu/background.cc
  src/hw/ppu/merge.cc
  src/hw/ppu/ppu.cc
  src/hw/ppu/registers.cc
  src/hw/ppu/serialization.cc
  src/hw/ppu/sprite.cc
  src/hw/ppu/window.cc
  src/hw/rom/backup/eeprom.cc
  src/hw/rom/backup/flash.cc
  src/hw/rom/backup/serialization.cc
  src/hw/rom/backup/sram.cc
  src/hw/rom/gpio/gpio.cc
  src/hw/rom/gpio/rtc.cc
  src/hw/rom/gpio/serialization.cc
  src/hw/rom/gpio/solar_sensor.cc
  src/hw/dma/dma.cc
  src/hw/dma/serialization.cc
  src/hw/irq/irq.cc
  src/hw/irq/serialization.cc
  src/hw/keypad/keypad.cc
  src/hw/keypad/serialization.cc
  src/hw/timer/serialization.cc
  src/hw/timer/timer.cc
  src/core.cc
  src/serialization.cc
)

set(HEADERS
  src/arm/handlers/arithmetic.inl
  src/arm/handlers/handler16.inl
  src/arm/handlers/handler32.inl
  src/arm/handlers/memory.inl
  src/arm/tablegen/gen_arm.hh
  src/arm/tablegen/gen_thumb.hh
  src/arm/arm7tdmi.hh
  src/arm/state.hh
  src/bus/bus.hh
  src/bus/io.hh
  src/hw/apu/channel/base_channel.hh
  src/hw/apu/channel/envelope.hh
  src/hw/apu/channel/fifo.hh
  src/hw/apu/channel/length_counter.hh
  src/hw/apu/channel/noise_channel.hh
  src/hw/apu/channel/quad_channel.hh
  src/hw/apu/channel/sweep.hh
  src/hw/apu/channel/wave_channel.hh
  src/hw/apu/hle/mp2k.hh
  src/hw/apu/apu.hh
  src/hw/apu/registers.hh
  src/hw/ppu/background.inl
  src/hw/ppu/ppu.hh
  src/hw/ppu/registers.hh
  src/hw/dma/dma.hh
  src/hw/irq/irq.hh
  src/hw/keypad/keypad.hh
  src/hw/timer/timer.hh
  src/core.hh
)

set(HEADERS_PUBLIC
  include/nba/common/dsp/resampler/cosine.hh
  include/nba/common/dsp/resampler/cubic.hh
  include/nba/common/dsp/resampler/nearest.hh
  include/nba/common/dsp/resampler/sinc.hh
  include/nba/common/dsp/resampler.hh
  include/nba/common/crc32.hh
  include/nba/common/meta.hh
  include/nba/common/punning.hh
  include/nba/common/scope_exit.hh
  include/nba/device/audio_device.hh
  include/nba/device/video_device.hh
  include/nba/rom/backup/backup.hh
  include/nba/rom/backup/backup_file.hh
  include/nba/rom/backup/eeprom.hh
  include/nba/rom/backup/flash.hh
  include/nba/rom/backup/sram.hh
  include/nba/rom/gpio/gpio.hh
  include/nba/rom/gpio/rtc.hh
  include/nba/rom/gpio/solar_sensor.hh
  include/nba/rom/header.hh
  include/nba/rom/rom.hh
  include/nba/config.hh
  include/nba/core.hh
  include/nba/integer.hh
  include/nba/log.hh
  include/nba/save_state.hh
  include/nba/scheduler.hh
)

add_library(nba STATIC)
target_sources(nba PRIVATE ${HEADERS_PUBLIC} ${HEADERS} ${SOURCES})
target_include_directories(nba PUBLIC include PRIVATE src)
target_link_libraries(nba PUBLIC fmt::fmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Disables some annoying warning about typeids being too evaluated™️
  target_compile_options(nba PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fbracket-depth=4096> PUBLIC -Wno-potentially-evaluated-expression)
endif()
