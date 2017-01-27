set(SOURCES
    cpu.cpp
    bios.cpp
    memory.cpp
    ppu/io.cpp
    ppu/ppu.cpp
)

set(HEADERS
    io.hpp
    cpu.hpp
    ppu/io.hpp
    ppu/ppu.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)
