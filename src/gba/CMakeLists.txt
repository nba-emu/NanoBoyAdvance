set(SOURCES
    cpu/apu/apu.cpp
    cpu/apu/regs.cpp
    cpu/cpu.cpp
    cpu/dma.cpp
    cpu/memory/mmio.cpp
    cpu/timer.cpp
    cpu/ppu/ppu.cpp
    cpu/ppu/regs.cpp
    cpu/ppu/render/text.cpp
    cpu/ppu/render/oam.cpp
    emulator.cpp
)

set(HEADERS
    cpu/apu/apu.hpp
    cpu/apu/regs.hpp
    cpu/cpu.hpp
    cpu/dma.hpp
    cpu/memory/mmio.hpp
    cpu/timer.hpp
    cpu/ppu/ppu.hpp
    cpu/ppu/regs.hpp
    cpu/scheduler.hpp
    emulator.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba arm stdc++fs)
