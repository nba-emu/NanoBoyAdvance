SET(SOURCES
    arm/arm7tdmi/armv4/emit.cpp
    gba/apu/apu.cpp
    gba/apu/regs.cpp
    gba/bios.cpp
    gba/mmio.cpp
    gba/ppu/regs.cpp
    gba/ppu/ppu.cpp
    gba/ppu/render/text.cpp
    gba/ppu/render/oam.cpp
    gba/cpu.cpp
    gba/dma.cpp
    gba/timer.cpp
)

SET(HEADERS
    arm/core/arm.hpp
    arm/core/interface.hpp
    gba/apu/apu.hpp
    gba/apu/regs.hpp
    gba/ppu/ppu.hpp
    gba/ppu/regs.hpp
    gba/cpu.hpp
    gba/event_device.hpp
    gba/mmio.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(core stdc++fs)