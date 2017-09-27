
set(ARM_SRC
    processor/arm/arm.cpp
    processor/arm/instr_arm.cpp
    processor/arm/instr_thumb.cpp
)

set(ARM_HDR
    processor/arm/arm.hpp
    processor/arm/alu.hpp
    processor/arm/inline_code.hpp
    processor/arm/instr_arm.hpp
    processor/arm/instr_thumb.hpp
    processor/arm/memory.hpp
)

set(EMULATOR_SRC
    system/gba/emulator.cpp
    system/gba/memory/mmio.cpp
    system/gba/dma/dma.cpp
    system/gba/dma/regs.cpp
    system/gba/timer/timer.cpp
    system/gba/timer/regs.cpp
)

set(EMULATOR_HDR
    system/gba/emulator.hpp
    system/gba/enums.hpp
    system/gba/interrupt.hpp
    system/gba/memory/mmio.hpp
    system/gba/memory/memory.hpp
    system/gba/dma/regs.hpp
    system/gba/timer/regs.hpp
)

set(CART_SRC
    system/gba/cart/sram.cpp
    system/gba/cart/flash.cpp
    system/gba/cart/eeprom.cpp
    system/gba/cart/cartridge.cpp
)

set(CART_HDR
    system/gba/cart/save.hpp
    system/gba/cart/sram.hpp
    system/gba/cart/flash.hpp
    system/gba/cart/eeprom.hpp
    system/gba/cart/header.hpp
    system/gba/cart/cartridge.hpp
)

set(PPU_SRC
    system/gba/ppu/io.cpp
    system/gba/ppu/ppu.cpp
    system/gba/ppu/composer.cpp
    system/gba/ppu/renderers/bitmap.cpp
    system/gba/ppu/renderers/text.cpp
    system/gba/ppu/renderers/obj.cpp
    system/gba/ppu/renderers/affine.cpp
)

set(PPU_HDR
    system/gba/ppu/io.hpp
    system/gba/ppu/enums.hpp
    system/gba/ppu/ppu.hpp
)

set(APU_SRC
    system/gba/apu/apu.cpp
    system/gba/apu/io.cpp
)

set(APU_HDR
    system/gba/apu/apu.hpp
    system/gba/apu/fifo.hpp
    system/gba/apu/io.hpp
)

set(SOURCES
    ${ARM_SRC}
    ${EMULATOR_SRC}
    ${CART_SRC}
    ${PPU_SRC}
    ${APU_SRC}
)

set(HEADERS
    system/gba/config.hpp
    ${ARM_HDR}
    ${EMULATOR_HDR}
    ${CART_HDR}
    ${PPU_HDR}
    ${APU_HDR}
)

add_library(core STATIC ${SOURCES} ${HEADERS})
target_link_libraries(core util)
