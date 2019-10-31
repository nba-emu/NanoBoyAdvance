set(SOURCES
    emulator.cpp
    system/apu/psg/channel_noise.cpp
    system/apu/psg/channel_quad.cpp
    system/apu/psg/channel_wave.cpp
    system/apu/apu.cpp
    system/apu/regs.cpp
    system/arm/isa-armv4/emit.cpp
    system/backup/eeprom.cpp
    system/cpu.cpp
    system/memory/registers.cpp
    system/dma.cpp
    system/timer.cpp
    system/ppu/ppu.cpp
    system/ppu/regs.cpp
    system/ppu/render/affine.cpp
    system/ppu/render/text.cpp
    system/ppu/render/oam.cpp
)

set(HEADERS
    emulator.hpp
    system/apu/apu.hpp
    system/apu/regs.hpp
    system/arm/arm7.hpp
    system/arm/interface.hpp
    system/arm/state.hpp
    system/backup/backup.hpp
    system/backup/eeprom.hpp
    system/cpu.hpp
    system/dma.hpp
    system/memory/registers.hpp
    system/timer.hpp
    system/ppu/ppu.hpp
    system/ppu/regs.hpp
    system/scheduler.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba stdc++fs)
