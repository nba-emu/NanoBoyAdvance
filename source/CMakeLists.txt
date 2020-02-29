include_directories(.)

option(PLATFORM_QT "Enable Qt5 frontend" ON)
option(PLATFORM_SDL "Enable SDL2 frontend" ON)

add_subdirectory(third_party)

set(SOURCES
  # Common
  common/log.cpp

  # Cartridge
  emulator/cartridge/backup/eeprom.cpp
  emulator/cartridge/backup/flash.cpp
  emulator/cartridge/game_db.cpp

  # Core
  emulator/core/apu/channel/channel_noise.cpp
  emulator/core/apu/channel/channel_quad.cpp
  emulator/core/apu/channel/channel_wave.cpp
  emulator/core/apu/apu.cpp
  emulator/core/apu/callback.cpp
  emulator/core/apu/registers.cpp
  emulator/core/arm/tablegen/tablegen.cpp
  emulator/core/dma/dma.cpp
  emulator/core/ppu/render/affine.cpp
  emulator/core/ppu/render/bitmap.cpp
  emulator/core/ppu/render/oam.cpp
  emulator/core/ppu/render/text.cpp
  emulator/core/ppu/render/window.cpp
  emulator/core/ppu/compose.cpp
  emulator/core/ppu/ppu.cpp
  emulator/core/timer/timer.cpp
  emulator/core/ppu/registers.cpp
  emulator/core/cpu.cpp
  emulator/core/registers.cpp

  # Emulator
  emulator/emulator.cpp
)

set(HEADERS
  # Common
  common/dsp/resampler/cosine.hpp
  common/dsp/resampler/cubic.hpp
  common/dsp/resampler/nearest.hpp
  common/dsp/resampler/windowed-sinc.hpp
  common/dsp/resampler.hpp
  common/framelimiter.hpp
  common/log.hpp
  common/static_for.hpp

  # Cartridge
  emulator/cartridge/backup/backup.hpp
  emulator/cartridge/backup/backup_file.hpp
  emulator/cartridge/backup/eeprom.hpp
  emulator/cartridge/backup/flash.hpp
  emulator/cartridge/backup/sram.hpp
  emulator/cartridge/game_db.hpp
  emulator/cartridge/header.hpp

  # Config
  emulator/config/config.hpp

  # Core
  emulator/core/apu/channel/channel_noise.hpp
  emulator/core/apu/channel/channel_quad.hpp
  emulator/core/apu/channel/channel_wave.hpp
  emulator/core/apu/channel/fifo.hpp
  emulator/core/apu/channel/sequencer.hpp
  emulator/core/apu/apu.hpp
  emulator/core/apu/registers.hpp
  emulator/core/arm/handlers/arithmetic.inl
  emulator/core/arm/handlers/handler16.inl
  emulator/core/arm/handlers/handler32.inl
  emulator/core/arm/handlers/memory.inl
  emulator/core/arm/tablegen/gen_arm.hpp
  emulator/core/arm/tablegen/gen_thumb.hpp
  emulator/core/arm/arm7tdmi.hpp
  emulator/core/arm/memory.hpp
  emulator/core/arm/state.hpp
  emulator/core/dma/dma.hpp
  emulator/core/ppu/helper.inl
  emulator/core/ppu/ppu.hpp
  emulator/core/ppu/registers.hpp
  emulator/core/timer/timer.hpp
  emulator/core/cpu.hpp
  emulator/core/interrupt.hpp
  emulator/core/memory.inl
  emulator/core/registers.hpp
  emulator/core/scheduler.hpp

  # Devices
  emulator/device/audio_device.hpp
  emulator/device/input_device.hpp
  emulator/device/video_device.hpp

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