cmake_minimum_required(VERSION 3.2)
project(nba CXX)

add_subdirectory(../../external ${CMAKE_BINARY_DIR}/external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/emulator/cartridge/backup/eeprom.cpp
  src/emulator/cartridge/backup/flash.cpp
  src/emulator/cartridge/backup/sram.cpp
  src/emulator/cartridge/gpio/gpio.cpp
  src/emulator/cartridge/gpio/rtc.cpp
  src/emulator/core/arm/tablegen/tablegen.cpp
  src/emulator/core/bus/bus.cpp
  src/emulator/core/bus/io.cpp
  src/emulator/core/bus/timing.cpp
  src/emulator/core/hw/apu/channel/noise_channel.cpp
  src/emulator/core/hw/apu/channel/quad_channel.cpp
  src/emulator/core/hw/apu/channel/wave_channel.cpp
  src/emulator/core/hw/apu/hle/mp2k.cpp
  src/emulator/core/hw/apu/apu.cpp
  src/emulator/core/hw/apu/callback.cpp
  src/emulator/core/hw/apu/registers.cpp
  src/emulator/core/hw/ppu/render/affine.cpp
  src/emulator/core/hw/ppu/render/bitmap.cpp
  src/emulator/core/hw/ppu/render/oam.cpp
  src/emulator/core/hw/ppu/render/text.cpp
  src/emulator/core/hw/ppu/render/window.cpp
  src/emulator/core/hw/ppu/compose.cpp
  src/emulator/core/hw/ppu/ppu.cpp
  src/emulator/core/hw/ppu/registers.cpp
  src/emulator/core/hw/dma.cpp
  src/emulator/core/hw/interrupt.cpp
  src/emulator/core/hw/keypad.cpp
  src/emulator/core/hw/timer.cpp
  src/emulator/core/core.cpp
)

set(HEADERS
  src/common/dsp/resampler/cosine.hpp
  src/common/dsp/resampler/cubic.hpp
  src/common/dsp/resampler/nearest.hpp
  src/common/dsp/resampler/sinc.hpp
  src/common/dsp/resampler.hpp
  src/common/crc32.hpp
  src/common/meta.hpp
  src/emulator/cartridge/gpio/rtc.hpp
  src/emulator/core/arm/handlers/arithmetic.inl
  src/emulator/core/arm/handlers/handler16.inl
  src/emulator/core/arm/handlers/handler32.inl
  src/emulator/core/arm/handlers/memory.inl
  src/emulator/core/arm/tablegen/gen_arm.hpp
  src/emulator/core/arm/tablegen/gen_thumb.hpp
  src/emulator/core/arm/arm7tdmi.hpp
  src/emulator/core/arm/state.hpp
  src/emulator/core/bus/bus.hpp
  src/emulator/core/bus/io.hpp
  src/emulator/core/hw/apu/channel/base_channel.hpp
  src/emulator/core/hw/apu/channel/envelope.hpp
  src/emulator/core/hw/apu/channel/fifo.hpp
  src/emulator/core/hw/apu/channel/length_counter.hpp
  src/emulator/core/hw/apu/channel/noise_channel.hpp
  src/emulator/core/hw/apu/channel/quad_channel.hpp
  src/emulator/core/hw/apu/channel/sweep.hpp
  src/emulator/core/hw/apu/channel/wave_channel.hpp
  src/emulator/core/hw/apu/hle/mp2k.hpp
  src/emulator/core/hw/apu/apu.hpp
  src/emulator/core/hw/apu/registers.hpp
  src/emulator/core/hw/ppu/helper.inl
  src/emulator/core/hw/ppu/ppu.hpp
  src/emulator/core/hw/ppu/registers.hpp
  src/emulator/core/hw/dma.hpp
  src/emulator/core/hw/interrupt.hpp
  src/emulator/core/hw/keypad.hpp
  src/emulator/core/hw/timer.hpp
  src/emulator/core/scheduler.hpp
)

set(HEADERS_PUBLIC
  include/nba/common/compiler.hpp
  include/nba/common/punning.hpp
  include/nba/deprecate/config.hpp
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
  include/nba/core.hpp
  include/nba/integer.hpp
  include/nba/log.hpp
)

add_library(nba STATIC ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(nba PRIVATE src)
target_include_directories(nba PUBLIC include)

target_link_libraries(nba PUBLIC fmt)