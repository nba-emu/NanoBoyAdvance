cmake_minimum_required(VERSION 3.2)
project(nba CXX)

add_subdirectory(../../external ${CMAKE_BINARY_DIR}/external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/arm/tablegen/tablegen.cpp
  src/bus/bus.cpp
  src/bus/io.cpp
  src/bus/timing.cpp
  src/hw/apu/channel/noise_channel.cpp
  src/hw/apu/channel/quad_channel.cpp
  src/hw/apu/channel/wave_channel.cpp
  src/hw/apu/hle/mp2k.cpp
  src/hw/apu/apu.cpp
  src/hw/apu/callback.cpp
  src/hw/apu/registers.cpp
  src/hw/ppu/render/affine.cpp
  src/hw/ppu/render/bitmap.cpp
  src/hw/ppu/render/oam.cpp
  src/hw/ppu/render/text.cpp
  src/hw/ppu/render/window.cpp
  src/hw/ppu/compose.cpp
  src/hw/ppu/ppu.cpp
  src/hw/ppu/registers.cpp
  src/hw/rom/backup/eeprom.cpp
  src/hw/rom/backup/flash.cpp
  src/hw/rom/backup/sram.cpp
  src/hw/rom/gpio/gpio.cpp
  src/hw/rom/gpio/rtc.cpp
  src/hw/dma.cpp
  src/hw/interrupt.cpp
  src/hw/keypad.cpp
  src/hw/timer.cpp
  src/core.cpp
)

set(HEADERS
  src/common/dsp/resampler/cosine.hpp
  src/common/dsp/resampler/cubic.hpp
  src/common/dsp/resampler/nearest.hpp
  src/common/dsp/resampler/sinc.hpp
  src/common/dsp/resampler.hpp
  src/common/crc32.hpp
  src/common/meta.hpp
  src/arm/handlers/arithmetic.inl
  src/arm/handlers/handler16.inl
  src/arm/handlers/handler32.inl
  src/arm/handlers/memory.inl
  src/arm/tablegen/gen_arm.hpp
  src/arm/tablegen/gen_thumb.hpp
  src/arm/arm7tdmi.hpp
  src/arm/state.hpp
  src/bus/bus.hpp
  src/bus/io.hpp
  src/hw/apu/channel/base_channel.hpp
  src/hw/apu/channel/envelope.hpp
  src/hw/apu/channel/fifo.hpp
  src/hw/apu/channel/length_counter.hpp
  src/hw/apu/channel/noise_channel.hpp
  src/hw/apu/channel/quad_channel.hpp
  src/hw/apu/channel/sweep.hpp
  src/hw/apu/channel/wave_channel.hpp
  src/hw/apu/hle/mp2k.hpp
  src/hw/apu/apu.hpp
  src/hw/apu/registers.hpp
  src/hw/ppu/helper.inl
  src/hw/ppu/ppu.hpp
  src/hw/ppu/registers.hpp
  src/hw/rom/gpio/rtc.hpp
  src/hw/dma.hpp
  src/hw/interrupt.hpp
  src/hw/keypad.hpp
  src/hw/timer.hpp
  src/core.hpp
  src/scheduler.hpp
)

set(HEADERS_PUBLIC
  include/nba/common/compiler.hpp
  include/nba/common/punning.hpp
  include/nba/device/audio_device.hpp
  include/nba/device/input_device.hpp
  include/nba/device/video_device.hpp
  include/nba/rom/backup/backup.hpp
  include/nba/rom/backup/backup_file.hpp
  include/nba/rom/backup/eeprom.hpp
  include/nba/rom/backup/flash.hpp
  include/nba/rom/backup/sram.hpp
  include/nba/rom/gpio/gpio.hpp
  include/nba/rom/header.hpp
  include/nba/rom/rom.hpp
  include/nba/config.hpp
  include/nba/core.hpp
  include/nba/integer.hpp
  include/nba/log.hpp
)

add_library(nba STATIC ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(nba PRIVATE src)
target_include_directories(nba PUBLIC include)

target_link_libraries(nba PUBLIC fmt)