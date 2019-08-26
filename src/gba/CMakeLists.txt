set(SOURCES
    apu/apu.cpp
    apu/regs.cpp
    cpu.cpp
    dma/dma.cpp
    dma/regs.cpp
    memory/mmio.cpp
    timer/regs.cpp
    timer/timer.cpp
    emulator.cpp
    ppu/ppu.cpp
    ppu/regs.cpp
    ppu/render/text.cpp
    ppu/render/oam.cpp
)

set(HEADERS
    apu/apu.hpp
    apu/regs.hpp
    cpu.hpp
    event_device.hpp
    memory/mmio.hpp
    emulator.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba arm stdc++fs)
