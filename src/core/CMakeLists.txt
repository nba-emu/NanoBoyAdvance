
set(ARM_SRC
    processor/arm/arm.cpp
    processor/arm/instr_arm.cpp
    processor/arm/instr_thumb.cpp
)

set(ARM_HDR
    processor/arm/arm.hpp
    processor/arm/alu.hpp
    processor/arm/enums.hpp
    processor/arm/context.hpp
    processor/arm/inline_code.hpp
    processor/arm/instr_arm.hpp
    processor/arm/instr_thumb.hpp
    processor/arm/tables/op_arm.hpp
    processor/arm/tables/op_thumb.hpp
)

set(EMULATOR_SRC
    emulator/emulator.cpp
    emulator/memory/mmio.cpp
    emulator/dma/dma.cpp
    emulator/dma/regs.cpp
    emulator/timer/timer.cpp
    emulator/timer/regs.cpp
)

set(EMULATOR_HDR
    emulator/emulator.hpp
    emulator/enums.hpp
    emulator/interrupt.hpp
    emulator/memory/mmio.hpp
    emulator/memory/memory.hpp
    emulator/dma/regs.hpp
    emulator/timer/regs.hpp
)

set(CART_SRC
    cart/sram.cpp
    cart/flash.cpp
    cart/eeprom.cpp
    cart/cartridge.cpp
)

set(CART_HDR
    cart/save.hpp
    cart/sram.hpp
    cart/flash.hpp
    cart/eeprom.hpp
    cart/header.hpp
    cart/cartridge.hpp
)

set(PPU_SRC
    ppu/io.cpp
    ppu/ppu.cpp
    ppu/composer.cpp
    ppu/renderers/bitmap.cpp
    ppu/renderers/text.cpp
    ppu/renderers/obj.cpp
    ppu/renderers/affine.cpp
)

set(PPU_HDR
    ppu/io.hpp
    ppu/enums.hpp
    ppu/ppu.hpp
)

set(APU_SRC
    apu/apu.cpp
    apu/io.cpp
)

set(APU_HDR
    apu/apu.hpp
    apu/fifo.hpp
    apu/io.hpp
)

set(SOURCES
    ${ARM_SRC}
    ${EMULATOR_SRC}
    ${CART_SRC}
    ${PPU_SRC}
    ${APU_SRC}
)

set(HEADERS
    config.hpp
    ${ARM_HDR}
    ${EMULATOR_HDR}
    ${CART_HDR}
    ${PPU_HDR}
    ${APU_HDR}
)

add_library(core STATIC ${SOURCES} ${HEADERS})
target_link_libraries(core util)
