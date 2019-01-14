SET(SOURCES
    arm/isa/thumb.cpp
    arm/isa/arm.cpp
    mmio.cpp
    ppu/io.cpp
    ppu/ppu.cpp
    ppu/render.cpp
    cpu.cpp
)

SET(HEADERS
    arm/arm.hpp
    arm/enums.hpp
    arm/interface.hpp
    arm/state.hpp
    arm/util.hpp
    ppu/ppu.hpp
    ppu/io.hpp
    cpu.hpp
    mmio.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
