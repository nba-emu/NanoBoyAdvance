cmake_minimum_required(VERSION 3.2)
project(nba CXX)

add_subdirectory(../../external ${CMAKE_BINARY_DIR}/external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/common/log.cpp
  src/emulator/cartridge/backup/eeprom.cpp
  src/emulator/cartridge/backup/flash.cpp
  src/emulator/cartridge/gpio/gpio.cpp
  src/emulator/cartridge/gpio/rtc.cpp
  src/emulator/cartridge/game_db.cpp
  src/emulator/core/arm/tablegen/tablegen.cpp
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
  src/emulator/core/hw/serial.cpp
  src/emulator/core/hw/timer.cpp
  src/emulator/core/cpu.cpp
  src/emulator/core/cpu-mmio.cpp
  src/emulator/emulator.cpp)

set(HEADERS
  src/common/dsp/resampler/cosine.hpp
  src/common/dsp/resampler/cubic.hpp
  src/common/dsp/resampler/nearest.hpp
  src/common/dsp/resampler/windowed-sinc.hpp
  src/common/dsp/resampler.hpp
  src/common/compiler.hpp
  src/common/crc32.hpp
  src/common/compiler.hpp
  src/common/log.hpp
  src/common/punning.hpp
  src/common/static_for.hpp
  src/emulator/cartridge/backup/backup.hpp
  src/emulator/cartridge/backup/backup_file.hpp
  src/emulator/cartridge/backup/eeprom.hpp
  src/emulator/cartridge/backup/flash.hpp
  src/emulator/cartridge/backup/sram.hpp
  src/emulator/cartridge/gpio/gpio.hpp
  src/emulator/cartridge/gpio/rtc.hpp
  src/emulator/cartridge/game_db.hpp
  src/emulator/cartridge/game_pak.hpp
  src/emulator/cartridge/header.hpp
  src/emulator/core/arm/handlers/arithmetic.inl
  src/emulator/core/arm/handlers/handler16.inl
  src/emulator/core/arm/handlers/handler32.inl
  src/emulator/core/arm/handlers/memory.inl
  src/emulator/core/arm/tablegen/gen_arm.hpp
  src/emulator/core/arm/tablegen/gen_thumb.hpp
  src/emulator/core/arm/arm7tdmi.hpp
  src/emulator/core/arm/memory.hpp
  src/emulator/core/arm/state.hpp
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
  src/emulator/core/hw/serial.hpp
  src/emulator/core/hw/timer.hpp
  src/emulator/core/cpu.hpp
  src/emulator/core/cpu-memory.inl
  src/emulator/core/cpu-mmio.hpp
  src/emulator/core/scheduler.hpp
  src/emulator/emulator.hpp
)

set(HEADERS_PUBLIC
  include/nba/device/audio_device.hpp
  include/nba/device/input_device.hpp
  include/nba/device/video_device.hpp
  include/nba/config.hpp
  include/nba/integer.hpp
)

add_library(nba STATIC ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})
target_include_directories(nba PRIVATE src)
target_include_directories(nba PUBLIC include)

# TODO: move the toml dependecy outside the NBA library?
target_link_libraries(nba PRIVATE fmt toml11::toml11)