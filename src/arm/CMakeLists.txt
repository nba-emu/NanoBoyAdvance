set(SOURCES
    arm7tdmi/isa-emit.cpp
)

set(HEADERS
    arm7tdmi/arm7tdmi.hpp
    common/arm.hpp
    common/interface.hpp
)

add_library(arm STATIC ${SOURCES} ${HEADERS})