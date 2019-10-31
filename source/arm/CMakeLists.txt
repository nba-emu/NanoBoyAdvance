set(SOURCES
    isa-armv4/emit.cpp
)

set(HEADERS
    arm7.hpp
    interface.hpp
    state.hpp
)

add_library(arm STATIC ${SOURCES} ${HEADERS})