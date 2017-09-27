
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
    emulator/cart/sram.cpp
    emulator/cart/flash.cpp
    emulator/cart/eeprom.cpp
    emulator/cart/cartridge.cpp
)

set(CART_HDR
    emulator/cart/save.hpp
    emulator/cart/sram.hpp
    emulator/cart/flash.hpp
    emulator/cart/eeprom.hpp
    emulator/cart/header.hpp
    emulator/cart/cartridge.hpp
)

set(PPU_SRC
    emulator/ppu/io.cpp
    emulator/ppu/ppu.cpp
    emulator/ppu/composer.cpp
    emulator/ppu/renderers/bitmap.cpp
    emulator/ppu/renderers/text.cpp
    emulator/ppu/renderers/obj.cpp
    emulator/ppu/renderers/affine.cpp
)

set(PPU_HDR
    emulator/ppu/io.hpp
    emulator/ppu/enums.hpp
    emulator/ppu/ppu.hpp
)

set(APU_SRC
    emulator/apu/apu.cpp
    emulator/apu/io.cpp
)

set(APU_HDR
    emulator/apu/apu.hpp
    emulator/apu/fifo.hpp
    emulator/apu/io.hpp
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
