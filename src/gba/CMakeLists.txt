set(SOURCES
    cpu/io.cpp
    cpu/cpu.cpp
    cpu/bios.cpp
    cpu/memory.cpp
    cpu/mmio.cpp
    cpu/timer.cpp
    cpu/dma.cpp
    cart/flash.cpp
    ppu/io.cpp
    ppu/ppu.cpp
    ppu/composer.cpp
    ppu/renderers/bitmap.cpp
    ppu/renderers/text.cpp
    ppu/renderers/obj.cpp
    ppu/renderers/affine.cpp
    apu/apu.cpp
    apu/io.cpp
)

set(HEADERS
    config.hpp
    cpu/io.hpp
    cpu/cpu.hpp
    cpu/enums.hpp
    cpu/interrupt.hpp
    cpu/mmio.hpp
    cpu/memory.hpp
    cart/cart_backup.hpp
    cart/flash.hpp
    ppu/io.hpp
    ppu/enums.hpp
    ppu/ppu.hpp
    apu/apu.hpp
    apu/fifo.hpp
    apu/io.hpp
)

add_library(gba STATIC ${SOURCES} ${HEADERS})
target_link_libraries(gba armigo util)
