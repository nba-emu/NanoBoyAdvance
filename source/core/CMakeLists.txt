set(SOURCES
    ../emulator/emulator.cpp
    arm/tablegen/tablegen.cpp
    apu/channel/channel_noise.cpp
    apu/channel/channel_quad.cpp
    apu/channel/channel_wave.cpp
    apu/apu.cpp
    apu/callback.cpp
    apu/registers.cpp
    backup/eeprom.cpp
    backup/flash.cpp
    cpu.cpp
    memory/registers.cpp
    dma.cpp
    timer.cpp
    ppu/compose.cpp
    ppu/ppu.cpp
    ppu/registers.cpp
    ppu/render/affine.cpp
    ppu/render/bitmap.cpp
    ppu/render/text.cpp
    ppu/render/oam.cpp
    ppu/render/window.cpp
)

set(HEADERS
    ../emulator/emulator.hpp
    apu/apu.hpp
    apu/registers.hpp
    arm/arm7tdmi.hpp
    arm/memory.hpp
    arm/state.hpp
    backup/backup.hpp
    backup/eeprom.hpp
    backup/flash.hpp
    cpu.hpp
    dma.hpp
    memory/registers.hpp
    timer.hpp
    ppu/ppu.hpp
    ppu/registers.hpp
    scheduler.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(gba stdc++fs)
endif()
