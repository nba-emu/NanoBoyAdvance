set(SOURCES
    arm.cpp
    arm/arm_emu.cpp
    thumb/thumb_emu.cpp
)

set(HEADERS
    arm.hpp
    enums.hpp
    flags.hpp
    memory.hpp
    shifting.hpp
    arm/arm_emu.hpp
    arm/arm_lut.hpp
    thumb/thumb_emu.hpp
    thumb/thumb_lut.hpp
)

add_library(armigo STATIC ${SOURCES} ${HEADERS})
