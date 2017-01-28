set(SOURCES
    io.cpp
    cpu.cpp
    bios.cpp
    interrupt.cpp
    memory.cpp
    mmio.cpp
    ppu/io.cpp
    ppu/ppu.cpp
    ppu/composer.cpp
    ppu/renderers/bitmap.cpp
    ppu/renderers/textmode.cpp
)

set(HEADERS
    io.hpp
    cpu.hpp
    interrupt.hpp
    mmio.hpp
    ppu/io.hpp
    ppu/ppu.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)
