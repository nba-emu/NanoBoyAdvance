set(SOURCES
    apu/apu.cpp
    apu/regs.cpp
    cpu/cpu.cpp
    cpu/dma.cpp
    cpu/memory/mmio.cpp
    cpu/timer.cpp
    emulator.cpp
    ppu/ppu.cpp
    ppu/regs.cpp
    ppu/render/text.cpp
    ppu/render/oam.cpp
)

set(HEADERS
    apu/apu.hpp
    apu/regs.hpp
    cpu/cpu.hpp
    cpu/dma.hpp
    cpu/memory/mmio.hpp
    cpu/timer.hpp
    emulator.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
    cpu/scheduler.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba arm stdc++fs)
