include_directories(.)

option(PLATFORM_QT "Enable Qt5 frontend" ON)
option(PLATFORM_SDL "Enable SDL2 frontend" ON)

add_subdirectory(third_party)

set(SOURCES
  # Cartridge
  cartridge/backup/eeprom.cpp
  cartridge/backup/flash.cpp
  cartridge/game_db.cpp

  # Common
  common/log.cpp

  # APU
  core/apu/channel/channel_noise.cpp
  core/apu/channel/channel_quad.cpp
  core/apu/channel/channel_wave.cpp
  core/apu/apu.cpp
  core/apu/callback.cpp
  core/apu/registers.cpp

  # ARM
  core/arm/tablegen/tablegen.cpp

  # Memory
  core/memory/registers.cpp

  # PPU
  core/ppu/render/affine.cpp
  core/ppu/render/bitmap.cpp
  core/ppu/render/oam.cpp
  core/ppu/render/text.cpp
  core/ppu/render/window.cpp
  core/ppu/compose.cpp
  core/ppu/ppu.cpp
  core/ppu/registers.cpp

  # CPU, DMA, Timer
  core/cpu.cpp
  core/dma.cpp
  core/timer.cpp

  # Emulator
  emulator/emulator.cpp
)

set(HEADERS
  # Cartridge
  cartridge/backup/backup.hpp
  cartridge/backup/backup_file.hpp
  cartridge/backup/eeprom.hpp
  cartridge/backup/flash.hpp
  cartridge/backup/sram.hpp
  cartridge/game_db.hpp
  cartridge/header.hpp

  # Common
  common/dsp/resampler/cosine.hpp
  common/dsp/resampler/cubic.hpp
  common/dsp/resampler/nearest.hpp
  common/dsp/resampler/windowed-sinc.hpp
  common/dsp/resampler.hpp
  common/framelimiter.hpp
  common/log.hpp
  common/static_for.hpp

  # Config
  config/config.hpp

  # APU
  core/apu/channel/channel_noise.hpp
  core/apu/channel/channel_quad.hpp
  core/apu/channel/channel_wave.hpp
  core/apu/channel/fifo.hpp
  core/apu/channel/sequencer.hpp
  core/apu/apu.hpp
  core/apu/registers.hpp

  # ARM
  core/arm/handlers/arithmetic.inl
  core/arm/handlers/handler16.inl
  core/arm/handlers/handler32.inl
  core/arm/handlers/memory.inl
  core/arm/tablegen/gen_arm.hpp
  core/arm/tablegen/gen_thumb.hpp
  core/arm/arm7tdmi.hpp
  core/arm/memory.hpp
  core/arm/state.hpp

  # Memory
  core/memory/memory.inl
  core/memory/registers.hpp

  # PPU
  core/ppu/helper.inl
  core/ppu/ppu.hpp
  core/ppu/registers.hpp

  # CPU, DMA, Timer
  core/cpu.hpp
  core/dma.hpp
  core/scheduler.hpp
  core/timer.hpp

  # Devices
  device/audio_device.hpp
  device/input_device.hpp
  device/video_device.hpp

  # Emulator
  emulator/emulator.hpp
)

add_library(nba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(nba fmt)

# TODO: this is not really optimal.
# What do we do about it?
if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(nba stdc++fs)
endif()


if (PLATFORM_QT)
  add_subdirectory("platform/qt")
endif()

if (PLATFORM_SDL)
  add_subdirectory("platform/sdl")
endif()