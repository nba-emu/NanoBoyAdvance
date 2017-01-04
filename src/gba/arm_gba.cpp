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

    void arm_gba::software_interrupt(int number)
    {
        switch (number)
        {
        case 0x06:
        {
            if (m_reg[1] != 0)
            {
                u32 mod = m_reg[0] % m_reg[1];
                u32 div = m_reg[0] / m_reg[1];
                m_reg[0] = div;
                m_reg[1] = mod;
            } else
            {
                LOG(LOG_ERROR, "SWI6h: Attempted division by zero.");
            }
            break;
        }
        case 0x05:
            m_reg[0] = 1;
            m_reg[1] = 1;
        case 0x04:
            Interrupt::WriteMasterEnableLow(1);
            Interrupt::WriteMasterEnableHigh(0);

            // If r0 is one IF must be cleared
            if (m_reg[0] == 1)
            {
                Interrupt::WriteInterruptFlagLow(0);
                Interrupt::WriteInterruptFlagHigh(0);
            }

            // Sets GBA into halt state, waiting for specific interrupt(s) to occur.
            Memory::m_IntrWait = true;
            Memory::m_IntrWaitMask = m_reg[1];
            Memory::m_HaltState = HALTSTATE_HALT;
            break;
        case 0x0B:
        {
            u32 source = m_reg[0];
            u32 dest = m_reg[1];
            u32 length = m_reg[2] & 0xFFFFF;
            bool fixed = m_reg[2] & (1 << 24) ? true : false;

            if (m_reg[2] & (1 << 26))
            {
                for (u32 i = 0; i < length; i++)
                {
                    Memory::WriteWord(dest & ~3, Memory::ReadWord(source & ~3));
                    dest += 4;

                    if (!fixed) source += 4;
                }
            }
            else
            {
                for (u32 i = 0; i < length; i++)
                {
                    Memory::WriteHWord(dest & ~1, Memory::ReadHWord(source & ~1));
                    dest += 2;

                    if (!fixed) source += 2;
                }
            }
            break;
        }
        case 0x0C:
        {
            u32 source = m_reg[0];
            u32 dest = m_reg[1];
            u32 length = m_reg[2] & 0xFFFFF;
            bool fixed = m_reg[2] & (1 << 24) ? true : false;

            for (u32 i = 0; i < length; i++)
            {
                Memory::WriteWord(dest & ~3, Memory::ReadWord(source & ~3));
                dest += 4;
                if (!fixed) source += 4;
            }
            break;
        }
        case 0x11:
        case 0x12:
        {
            int amount = Memory::ReadWord(m_reg[0]) >> 8;
            u32 source = m_reg[0] + 4;
            u32 dest = m_reg[1];

            while (amount > 0)
            {
                u8 encoder = Memory::ReadByte(source++);

                // Process 8 blocks encoded by the encoder
                for (int i = 7; i >= 0; i--)
                {
                    if (encoder & (1 << i))
                    {
                        // Compressed
                        u16 value = Memory::ReadHWord(source);
                        u32 disp = (value >> 8) | ((value & 0xF) << 8);
                        u32 n = ((value >> 4) & 0xF) + 3;
                        source += 2;

                        for (u32 j = 0; j < n; j++)
                        {
                            u16 value = Memory::ReadByte(dest - disp - 1);
                            Memory::WriteHWord(dest, value);
                            dest++;

                            if (--amount == 0) return;
                        }
                    }
                    else
                    {
                        // Uncompressed
                        u8 value = Memory::ReadByte(source++);
                        Memory::WriteHWord(dest++, value);

                        if (--amount == 0) return;
                    }
                }
            }
            break;
        }
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Unimplemented software interrupt 0x%x", number);
            #endif
            break;
        }
    }
}
