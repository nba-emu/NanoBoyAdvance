set(SOURCES
    emulator.cpp
    system/apu/apu.cpp
    system/apu/regs.cpp
    system/cpu.cpp
    system/dma.cpp
    system/memory/mmio.cpp
    system/timer.cpp
    system/ppu/ppu.cpp
    system/ppu/regs.cpp
    system/ppu/render/text.cpp
    system/ppu/render/oam.cpp
)

set(HEADERS
    emulator.hpp
    system/apu/apu.hpp
    system/apu/regs.hpp
    system/cpu.hpp
    system/dma.hpp
    system/memory/mmio.hpp
    system/timer.hpp
    system/ppu/ppu.hpp
    system/ppu/regs.hpp
    system/scheduler.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba arm stdc++fs)
