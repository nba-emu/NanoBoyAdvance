# TODO: seperate lists for each folder/subsystem

set(ARM_SRC
    arm/arm.cpp
    arm/instr_arm.cpp
    arm/instr_thumb.cpp
)

set(ARM_HDR
    arm/arm.hpp
    arm/enums.hpp
    arm/context.hpp
    arm/inline_code.hpp
    arm/instr_arm.hpp
    arm/instr_thumb.hpp
    arm/tables/op_arm.hpp
    arm/tables/op_thumb.hpp
)

set(SOURCES
    ${ARM_SRC}
    emulator/io.cpp
    emulator/emulator.cpp
    emulator/mmio.cpp
    emulator/timer.cpp
    emulator/dma.cpp
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
    emulator/io.hpp
    emulator/emulator.hpp
    emulator/enums.hpp
    emulator/interrupt.hpp
    emulator/mmio.hpp
    emulator/memory.hpp
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
