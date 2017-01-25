set(SOURCES
    cpu.cpp
    ppu.cpp
)

set(HEADERS
    cpu.hpp
    ppu.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)
