#include "arm_gba.hpp"
#include "memory.h"

namespace GBA
{
    u8 arm_gba::bus_read_byte(u32 address)
    {
        return Memory::ReadByte(address);
    }

    u16 arm_gba::bus_read_hword(u32 address)
    {
        return Memory::ReadHWord(address);
    }

    u32 arm_gba::bus_read_word(u32 address)
    {
        return Memory::ReadWord(address);
    }

    void arm_gba::bus_write_byte(u32 address, u8 value)
    {
        Memory::WriteByte(address, value);
    }

    void arm_gba::bus_write_hword(u32 address, u16 value)
    {
        Memory::WriteHWord(address, value);
    }

    void arm_gba::bus_write_word(u32 address, u32 value)
    {
        Memory::WriteWord(address, value);
    }
}
