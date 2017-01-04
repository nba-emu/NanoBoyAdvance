#include "arm/arm.hpp"

#pragma once

namespace GBA
{
    class arm_gba : public arm
    {
    protected:
        u8 bus_read_byte(u32 address);
        u16 bus_read_hword(u32 address);
        u32 bus_read_word(u32 address);
        void bus_write_byte(u32 address, u8 value);
        void bus_write_hword(u32 address, u16 value);
        void bus_write_word(u32 address, u32 value);
        
        void software_interrupt(int number);
    };
}
