SET(SOURCES
    apu/apu.cpp
    apu/regs.cpp
    arm/isa/thumb.cpp
    arm/isa/arm.cpp
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
    arm/arm.hpp
    arm/enums.hpp
    arm/interface.hpp
    arm/state.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
    cpu.hpp
    event_device.hpp
    mmio.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(core stdc++fs)