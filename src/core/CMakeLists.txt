SET(SOURCES
    arm/isa/thumb.cpp
    arm/isa/arm.cpp
    mmio.cpp
    ppu/regs.cpp
    ppu/ppu.cpp
    ppu/render/text.cpp
    cpu.cpp
)

SET(HEADERS
    arm/arm.hpp
    arm/enums.hpp
    arm/interface.hpp
    arm/state.hpp
    arm/util.hpp
    ppu/ppu.hpp
    ppu/regs.hpp
    cpu.hpp
    mmio.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
