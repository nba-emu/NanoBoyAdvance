set(SOURCES
    emulator.cpp
    core/apu/channel/channel_noise.cpp
    core/apu/channel/channel_quad.cpp
    core/apu/channel/channel_wave.cpp
    core/apu/apu.cpp
    core/apu/callback.cpp
    core/apu/registers.cpp
    core/backup/eeprom.cpp
    core/backup/flash.cpp
    core/cpu.cpp
    core/memory/registers.cpp
    core/dma.cpp
    core/timer.cpp
    core/ppu/compose.cpp
    core/ppu/ppu.cpp
    core/ppu/registers.cpp
    core/ppu/render/affine.cpp
    core/ppu/render/bitmap.cpp
    core/ppu/render/text.cpp
    core/ppu/render/oam.cpp
    core/ppu/render/window.cpp
)

set(HEADERS
    emulator.hpp
    core/apu/apu.hpp
    core/apu/registers.hpp
    core/arm/arm7.hpp
    core/arm/interface.hpp
    core/arm/state.hpp
    core/backup/backup.hpp
    core/backup/eeprom.hpp
    core/backup/flash.hpp
    core/cpu.hpp
    core/dma.hpp
    core/memory/registers.hpp
    core/timer.hpp
    core/ppu/ppu.hpp
    core/ppu/registers.hpp
    core/scheduler.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(gba stdc++fs)
endif()
