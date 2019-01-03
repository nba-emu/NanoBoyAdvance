SET(SOURCES
    arm/isa/thumb.cpp
    arm/isa/arm.cpp
    mmio.cpp
    ppu/io.cpp
)

SET(HEADERS
    arm/debugger/breakpoint.hpp
    arm/debugger/debugger.hpp
    arm/arm.hpp
    arm/enums.hpp
    arm/interface.hpp
    arm/state.hpp
    arm/util.hpp
    cpu.hpp
    emulator.hpp
)

ADD_LIBRARY(core STATIC ${SOURCES} ${HEADERS})
