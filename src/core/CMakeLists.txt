SET(SOURCES
    apu/apu.cpp
    apu/regs.cpp
    arm/arm7tdmi/isa/thumb.cpp
    arm/arm7tdmi/isa/arm.cpp
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

SET(HEADERS
    apu/apu.hpp
    apu/regs.hpp
    arm/core/arm.hpp
    arm/core/interface.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
    cpu.hpp
    event_device.hpp
    mmio.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(core stdc++fs)