set(SOURCES
    arm7tdmi/armv4/emit.cpp
)

set(HEADERS
    arm7tdmi/arm7tdmi.hpp
    core/arm.hpp
    core/interface.hpp
)

add_library(arm STATIC ${SOURCES} ${HEADERS})