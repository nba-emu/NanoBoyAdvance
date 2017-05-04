# TODO: seperate lists for each folder/subsystem

set(ARM_SRC
    arm/arm.cpp
    arm/instr_arm.cpp
    arm/instr_thumb.cpp
)

set(ARM_HDR
    arm/arm.hpp
    arm/enums.hpp
    arm/memory.hpp
    arm/context.hpp
    arm/inline_code.hpp
    arm/instr_arm.hpp
    arm/instr_thumb.hpp
    arm/tables/op_arm.hpp
    arm/tables/op_thumb.hpp
)

set(SOURCES
    ${ARM_SRC}
    cpu/io.cpp
    cpu/cpu.cpp
    cpu/memory.cpp
    cpu/mmio.cpp
    cpu/timer.cpp
    cpu/dma.cpp
    cart/sram.cpp
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
    ${ARM_HDR}
    cpu/io.hpp
    cpu/cpu.hpp
    cpu/enums.hpp
    cpu/interrupt.hpp
    cpu/mmio.hpp
    cpu/memory.hpp
    cart/cart_backup.hpp
    cart/sram.hpp
    cart/flash.hpp
    ppu/io.hpp
    ppu/enums.hpp
    ppu/ppu.hpp
    apu/apu.hpp
    apu/fifo.hpp
    apu/io.hpp
)

add_library(core STATIC ${SOURCES} ${HEADERS})
target_link_libraries(core util)
