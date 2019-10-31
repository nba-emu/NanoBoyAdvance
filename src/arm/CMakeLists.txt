set(SOURCES
    isa-armv4/emit.cpp
)

set(HEADERS
    arm7tdmi.hpp
    interface.hpp
    state.hpp
)

add_library(arm STATIC ${SOURCES} ${HEADERS})