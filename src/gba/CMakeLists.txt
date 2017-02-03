set(SOURCES
    io.cpp
    cpu.cpp
    bios.cpp
    memory.cpp
    mmio.cpp
    timer.cpp
    dma.cpp
    flash.cpp
    ppu/io.cpp
    ppu/ppu.cpp
    ppu/composer.cpp
    ppu/renderers/bitmap.cpp
    ppu/renderers/textmode.cpp
    ppu/renderers/sprites.cpp
)

set(HEADERS
    io.hpp
    cpu.hpp
    enums.hpp
    interrupt.hpp
    mmio.hpp
    cart_backup.hpp
    flash.hpp
    ppu/io.hpp
    ppu/ppu.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)
