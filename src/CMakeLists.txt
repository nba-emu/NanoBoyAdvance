
add_subdirectory(../external ${CMAKE_CURRENT_BINARY_DIR}/external)

set(SOURCES
  # Common
  common/log.cpp

  # Cartridge
  emulator/cartridge/backup/eeprom.cpp
  emulator/cartridge/backup/flash.cpp
  emulator/cartridge/gpio/gpio.cpp
  emulator/cartridge/gpio/rtc.cpp
  emulator/cartridge/game_db.cpp

  # Config
  emulator/config/config_toml.cpp

  # Core
  emulator/core/arm/tablegen/tablegen.cpp
  emulator/core/hw/apu/channel/noise_channel.cpp
  emulator/core/hw/apu/channel/quad_channel.cpp
  emulator/core/hw/apu/channel/wave_channel.cpp
  emulator/core/hw/apu/apu.cpp
  emulator/core/hw/apu/callback.cpp
  emulator/core/hw/apu/registers.cpp
  emulator/core/hw/ppu/render/affine.cpp
  emulator/core/hw/ppu/render/bitmap.cpp
  emulator/core/hw/ppu/render/oam.cpp
  emulator/core/hw/ppu/render/text.cpp
  emulator/core/hw/ppu/render/window.cpp
  emulator/core/hw/ppu/compose.cpp
  emulator/core/hw/ppu/ppu.cpp
  emulator/core/hw/ppu/registers.cpp
  emulator/core/hw/dma.cpp
  emulator/core/hw/interrupt.cpp
  emulator/core/hw/serial.cpp
  emulator/core/hw/timer.cpp
  emulator/core/cpu.cpp
  emulator/core/cpu-mmio.cpp

  # Emulator
  emulator/emulator.cpp)

set(HEADERS
  # Common
  common/dsp/resampler/cosine.hpp
  common/dsp/resampler/cubic.hpp
  common/dsp/resampler/nearest.hpp
  common/dsp/resampler/windowed-sinc.hpp
  common/dsp/resampler.hpp
  common/compiler.hpp
  common/integer.hpp
  common/compiler.hpp
  common/log.hpp
  common/punning.hpp
  common/static_for.hpp

  # Cartridge
  emulator/cartridge/backup/backup.hpp
  emulator/cartridge/backup/backup_file.hpp
  emulator/cartridge/backup/eeprom.hpp
  emulator/cartridge/backup/flash.hpp
  emulator/cartridge/backup/sram.hpp
  emulator/cartridge/gpio/gpio.hpp
  emulator/cartridge/gpio/rtc.hpp
  emulator/cartridge/game_db.hpp
  emulator/cartridge/game_pak.hpp
  emulator/cartridge/header.hpp

  # Config
  emulator/config/config.hpp
  emulator/config/config_toml.hpp

  # Core
  emulator/core/arm/handlers/arithmetic.inl
  emulator/core/arm/handlers/handler16.inl
  emulator/core/arm/handlers/handler32.inl
  emulator/core/arm/handlers/memory.inl
  emulator/core/arm/tablegen/gen_arm.hpp
  emulator/core/arm/tablegen/gen_thumb.hpp
  emulator/core/arm/arm7tdmi.hpp
  emulator/core/arm/memory.hpp
  emulator/core/arm/state.hpp
  emulator/core/hw/apu/channel/base_channel.hpp
  emulator/core/hw/apu/channel/envelope.hpp
  emulator/core/hw/apu/channel/fifo.hpp
  emulator/core/hw/apu/channel/length_counter.hpp
  emulator/core/hw/apu/channel/noise_channel.hpp
  emulator/core/hw/apu/channel/quad_channel.hpp
  emulator/core/hw/apu/channel/sweep.hpp
  emulator/core/hw/apu/channel/wave_channel.hpp
  emulator/core/hw/apu/apu.hpp
  emulator/core/hw/apu/registers.hpp
  emulator/core/hw/ppu/helper.inl
  emulator/core/hw/ppu/ppu.hpp
  emulator/core/hw/ppu/registers.hpp
  emulator/core/hw/dma.hpp
  emulator/core/hw/interrupt.hpp
  emulator/core/hw/serial.hpp
  emulator/core/hw/timer.hpp
  emulator/core/cpu.hpp
  emulator/core/cpu-memory.inl
  emulator/core/cpu-mmio.hpp
  emulator/core/scheduler.hpp

  # Devices
  emulator/device/audio_device.hpp
  emulator/device/input_device.hpp
  emulator/device/video_device.hpp

  # Emulator
  emulator/emulator.hpp)

add_library(nba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(nba fmt toml11::toml11)
target_include_directories(nba PUBLIC .)

add_subdirectory("platform/sdl")
