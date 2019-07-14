set(SOURCES
    apu/apu.cpp
    apu/regs.cpp
    bios.cpp
    mmio.cpp
    ppu/regs.cpp
    ppu/ppu.cpp
    ppu/render/text.cpp
    ppu/render/oam.cpp
    cpu.cpp
    dma.cpp
    timer.cpp
)

set(HEADERS
    apu/apu.hpp
    apu/regs.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
    cpu.hpp
    event_device.hpp
    mmio.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba arm stdc++fs)
