/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#include "arm7.h"

#define THUMB_1 1
#define THUMB_2 2
#define THUMB_3 3
#define THUMB_4 4
#define THUMB_5 5
#define THUMB_6 6
#define THUMB_7 7
#define THUMB_8 8
#define THUMB_9 9
#define THUMB_10 10
#define THUMB_11 11
#define THUMB_12 12
#define THUMB_13 13
#define THUMB_14 14
#define THUMB_15 15
#define THUMB_16 16
#define THUMB_17 17
#define THUMB_18 18
#define THUMB_19 19

namespace NanoboyAdvance
{
    int ARM7::THUMBDecode(ushort instruction)
    {
        if ((instruction & 0xF800) < 0x1800)
        {
            // THUMB.1 Move shifted register
            return THUMB_1;
        }
        else if ((instruction & 0xF800) == 0x1800)
        {
            // THUMB.2 Add/subtract
            return THUMB_2;
        }
        else if ((instruction & 0xE000) == 0x2000)
        {
            // THUMB.3 Move/compare/add/subtract immediate
            return THUMB_3;
        }
        else if ((instruction & 0xFC00) == 0x4000)
        {
            // THUMB.4 ALU operations
            return THUMB_4;
        }
        else if ((instruction & 0xFC00) == 0x4400)
        {
            // THUMB.5 Hi register operations/branch exchange
            return THUMB_5;
        }
        else if ((instruction & 0xF800) == 0x4800)
        {
            // THUMB.6 PC-relative load
            return THUMB_6;
        }
        else if ((instruction & 0xF200) == 0x5000)
        {
            // THUMB.7 Load/store with register offset
            return THUMB_7;
        }
        else if ((instruction & 0xF200) == 0x5200)
        {
            // THUMB.8 Load/store sign-extended byte/halfword
            return THUMB_8;
        }
        else if ((instruction & 0xE000) == 0x6000)
        {
            // THUMB.9 Load store with immediate offset
            return THUMB_9;
        }
        else if ((instruction & 0xF000) == 0x8000)
        {
            // THUMB.10 Load/store halfword
            return THUMB_10;
        }
        else if ((instruction & 0xF000) == 0x9000)
        {
            // THUMB.11 SP-relative load/store
            return THUMB_11;
        }
        else if ((instruction & 0xF000) == 0xA000)
        {
            // THUMB.12 Load address
            return THUMB_12;
        }
        else if ((instruction & 0xFF00) == 0xB000)
        {
            // THUMB.13 Add offset to stack pointer
            return THUMB_13;
        }
        else if ((instruction & 0xF600) == 0xB400)
        {
            // THUMB.14 push/pop registers
            return THUMB_14;
        }
        else if ((instruction & 0xF000) == 0xC000)
        {
            // THUMB.15 Multiple load/store
            return THUMB_15;
        }
        else if ((instruction & 0xF000) == 0xD000)
        {
            // THUMB.16 Conditional Branch
            return THUMB_16;
        }
        else if ((instruction & 0xFF00) == 0xDF00)
        {
            // THUMB.17 Software Interrupt
            return THUMB_17;
        }
        else if ((instruction & 0xF800) == 0xE000)
        {
            // THUMB.18 Unconditional Branch
            return THUMB_18;
        }
        else if ((instruction & 0xF000) == 0xF000)
        {
            // THUMB.19 Long branch with link
            return THUMB_19;
        }
        return 0;
    }

    void ARM7::THUMBExecute(ushort instruction, int type)
    {
#ifdef CPU_LOG
        // Log our status for debug reasons
        LOG(LOG_INFO, "([THUMB] Executing THUMB.%d, r15=0x%x", type, r15);
        for (int i = 0; i < 16; i++)
        {
            if (i == 15)
            {
                cout << "r" << i << " = 0x" << std::hex << reg(i) << " (0x" << (reg(i) - 4) << ")" << std::dec << endl;
            }
            else
            {
                cout << "r" << i << " = 0x" << std::hex << reg(i) << std::dec << endl;
            }
        }
        cout << "cpsr = 0x" << std::hex << cpsr << std::dec << endl;
        cout << "spsr = 0x" << std::hex << *pspsr << std::dec << endl;
        cout << "mode = ";
        switch (cpsr & 0x1F)
        {
        case User: cout << "User" << endl; break;
        case System: cout << "System" << endl; break;
        case IRQ: cout << "IRQ" << endl; break;
        case SVC: cout << "SVC" << endl; break;
        default: cout << "n.n." << endl; break;
        }
#endif 
        // Actual execution
        switch (type)
        {
        case THUMB_1:
        {
            // THUMB.1 Move shifted register
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            uint immediate_value = (instruction >> 6) & 0x1F;
            if (immediate_value != 0)
            {
                switch ((instruction >> 11) & 3)
                {
                case 0b00: // LSL
                    assert_carry((reg(reg_source) << (immediate_value - 1)) & 0x80000000);
                    reg(reg_dest) = reg(reg_source) << immediate_value;
                    break;
                case 0b01: // LSR
                    assert_carry((reg(reg_source) >> (immediate_value - 1)) & 1);
                    reg(reg_dest) = reg(reg_source) >> immediate_value;
                    break;
                case 0b10: // ASR
                {
                    sint result = (sint)reg(reg_source) >> (sint)immediate_value;
                    assert_carry((reg(reg_source) >> (immediate_value - 1)) & 1);
                    reg(reg_dest) = (uint)result;
                    break;
                }
                }
            }
            else
            {
                reg(reg_dest) = reg(reg_source);
            }
            calculate_sign(reg(reg_dest));
            calculate_zero(reg(reg_dest));
            break;
        }
        case THUMB_2:
        {
            // THUMB.2 Add/subtract
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            uint operand;

            // The operand can either be the value of a register or a 3 bit immediate value
            if (instruction & (1 << 10))
            {
                operand = (instruction >> 6) & 7;
            }
            else
            {
                operand = reg((instruction >> 6) & 7);
            }

            // Determine wether to subtract or add
            if (instruction & (1 << 9))
            {
                uint result = reg(reg_source) - operand;
                assert_carry(reg(reg_source) >= operand);
                calculate_overflow_sub(result, reg(reg_source), operand);
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
            }
            else
            {
                uint result = reg(reg_source) + operand;
                ulong result_long = (ulong)(reg(reg_source)) + (ulong)operand;
                assert_carry(result_long & 0x100000000);
                calculate_overflow_add(result, reg(reg_source), operand);
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
            }
            break;
        }
        case THUMB_3:
        {
            // THUMB.3 Move/compare/add/subtract immediate
            uint immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            switch ((instruction >> 11) & 3)
            {
            case 0b00: // MOV
                calculate_sign(0);
                calculate_zero(immediate_value);
                reg(reg_dest) = immediate_value;
                break;
            case 0b01: // CMP
            {
                uint result = reg(reg_dest) - immediate_value;
                assert_carry(reg(reg_dest) >= immediate_value);
                calculate_overflow_sub(result, reg(reg_dest), immediate_value);
                calculate_sign(result);
                calculate_zero(result);
                break;
            }
            case 0b10: // ADD
            {
                uint result = reg(reg_dest) + immediate_value;
                ulong result_long = (ulong)(reg(reg_dest)) + (ulong)immediate_value;
                assert_carry(result_long & 0x100000000);
                calculate_overflow_add(result, reg(reg_dest), immediate_value);
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b11: // SUB
            {
                uint result = reg(reg_dest) - immediate_value;
                assert_carry(reg(reg_dest) >= immediate_value);
                calculate_overflow_sub(result, reg(reg_dest), immediate_value);
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
                break;
            }
            }
            break;
        }
        // TODO: It seems like I was pretty tired when writing this... check and look for bugs carefully...
        case THUMB_4:
        {
            // THUMB.4 ALU operations
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            switch ((instruction >> 6) & 0xF)
            {
            case 0b0000: // AND
                reg(reg_dest) &= reg(reg_source);
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            case 0b0001: // EOR
                reg(reg_dest) ^= reg(reg_source);
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            case 0b0010: // LSL
            {
                uint amount = reg(reg_source);
                if (amount != 0)
                {
                    assert_carry((reg(reg_dest) << (amount - 1)) & 0x80000000);
                    reg(reg_dest) <<= amount;
                }
                else
                {
                    reg(reg_dest) = reg(reg_source);
                }
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            }
            case 0b0011: // LSR
            {
                uint amount = reg(reg_source);
                if (amount != 0)
                {
                    assert_carry((reg(reg_dest) >> (amount - 1)) & 1);
                    reg(reg_dest) >>= amount;
                }
                else
                {
                    reg(reg_dest) = reg(reg_source);
                }
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            }
            case 0b0100: // ASR
            {
                uint amount = reg(reg_source);
                if (amount != 0)
                {
                    sint result = (sint)(reg(reg_dest)) >> (sint)(reg(reg_source));
                    assert_carry((reg(reg_dest) >> (amount - 1)) & 1);
                    reg(reg_dest) = result;
                }
                else
                {
                    reg(reg_dest) = reg(reg_source);
                }
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            }
            case 0b0101: // ADC
            {
                int carry = (cpsr >> 29) & 1;
                uint result = reg(reg_dest) + reg(reg_source) + carry;
                ulong result_long = (ulong)(reg(reg_dest)) + (ulong)(reg(reg_source)) + (ulong)carry;
                assert_carry(result_long & 0x100000000);
                calculate_overflow_add(result, reg(reg_dest), reg(reg_source) + carry);
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b0110: // SBC
            {
                int carry = (cpsr >> 29) & 1;
                uint result = reg(reg_dest) - reg(reg_source) + carry - 1;
                assert_carry(reg(reg_dest) >= reg(reg_source) + carry - 1);
                calculate_overflow_sub(result, reg(reg_dest), (reg(reg_source) + carry - 1));
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b0111: // ROR
            {
                // TODO: Prevent reg_source from going higher that 32
                uint amount = reg(reg_source);
                if (amount != 0)
                {
                    uint result = (reg(reg_dest) >> amount) | (reg(reg_dest) << (32 - amount));
                    assert_carry((reg(reg_dest) >> (amount - 1)) & 1);
                    reg(reg_dest) = result;
                }
                else
                {
                    reg(reg_dest) = reg(reg_source);
                }
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            }
            case 0b1000: // TST
            {
                uint result = reg(reg_dest) & reg(reg_source);
                calculate_sign(result);
                calculate_zero(result);
                break;
            }
            case 0b1001: // NEG
            {
                uint result = 0 - reg(reg_source);
                assert_carry(0 >= reg(reg_source));
                calculate_overflow_sub(result, 0, reg(reg_source));
                calculate_sign(result);
                calculate_zero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b1010: // CMP
            {
                uint result = reg(reg_dest) - reg(reg_source);
                assert_carry(reg(reg_dest) >= reg(reg_source));
                calculate_overflow_sub(result, reg(reg_dest), reg(reg_source));
                calculate_sign(result);
                calculate_zero(result);
                break;
            }
            case 0b1011: // CMN
            {
                uint result = reg(reg_dest) + reg(reg_source);
                ulong result_long = (ulong)(reg(reg_dest)) + (ulong)(reg(reg_source));
                assert_carry(result_long & 0x100000000);
                calculate_overflow_add(result, reg(reg_dest), reg(reg_source));
                calculate_sign(result);
                calculate_zero(result);
                break;
            }
            case 0b1100: // ORR
                reg(reg_dest) |= reg(reg_source);
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            case 0b1101: // MUL
                reg(reg_dest) *= reg(reg_source);
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                assert_carry(false);
                break;
            case 0b1110: // BIC
                reg(reg_dest) &= ~(reg(reg_source));
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            case 0b1111: // MVN
                reg(reg_dest) = ~(reg(reg_source));
                calculate_sign(reg(reg_dest));
                calculate_zero(reg(reg_dest));
                break;
            }
            break;
        }
        case THUMB_5:
        {
            // THUMB.5 Hi register operations/branch exchange
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;

            // Both reg_dest and reg_source can encode either a low register (r0-r7) or a high register (r8-r15)
            switch ((instruction >> 6) & 3)
            {
            case 0b01:
                reg_source += 8;
                break;
            case 0b10:
                reg_dest += 8;
                break;
            case 0b11:
                reg_dest += 8;
                reg_source += 8;
                break;
            }

            // TODO: Handle special case that one of the registers is r15
            ASSERT(reg_dest == 15 || reg_source == 15, LOG_ERROR, "Hi register operations/branch exchange special case r15 not implemented, r15=0x%x", r15);

            // Perform the actual operation
            switch ((instruction >> 8) & 3)
            {
            case 0b00: // ADD
                reg(reg_dest) += reg(reg_source);
                break;
            case 0b01: // CMP
            {
                uint result = reg(reg_dest) - reg(reg_source);
                assert_carry(reg(reg_dest) >= reg(reg_source));
                calculate_overflow_sub(result, reg(reg_dest), reg(reg_source));
                calculate_sign(result);
                calculate_zero(result);
                break;
            }
            case 0b10: // MOV
                reg(reg_dest) = reg(reg_source);
                break;
            case 0b11: // BX
                if (reg(reg_source) & 1)
                {
                    r15 = reg(reg_source) & ~1;
                }
                else
                {
                    cpsr &= ~Thumb;
                    r15 = reg(reg_source) & ~3;
                }
                flush_pipe = true;
                break;
            }
            break;
        }
        case THUMB_6:
        {
            // THUMB.6 PC-relative load
            uint immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            // TODO: I'm pretty sure we don't even need the "Rotated" because address will always be aligned
            reg(reg_dest) = ReadWordRotated((r15 & ~2) + (immediate_value << 2));
            break;
        }
        case THUMB_7:
        {
            // THUMB.7 Load/store with register offset
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            int reg_offset = (instruction >> 6) & 7;
            uint address = reg(reg_base) + reg(reg_offset);
            switch ((instruction >> 10) & 3)
            {
            case 0b00: // STR
                WriteWord(address, reg(reg_dest));
                break;
            case 0b01: // STRB
                WriteByte(address, reg(reg_dest) & 0xFF);
                break;
            case 0b10: // LDR
                reg(reg_dest) = ReadWordRotated(address);
                break;
            case 0b11: // LDRB
                reg(reg_dest) = ReadByte(address);
                break;
            }
            break;
        }
        case THUMB_8:
        {
            // THUMB.8 Load/store sign-extended byte/halfword
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            int reg_offset = (instruction >> 6) & 7;
            uint address = reg(reg_base) + reg(reg_offset);
            switch ((instruction >> 10) & 3)
            {
            case 0b00: // STRH
                WriteHWord(address, reg(reg_dest));
                break;
            case 0b01: // LDSB
                reg(reg_dest) = ReadByte(address);
                if (reg(reg_dest) & 0x80)
                {
                    reg(reg_dest) |= 0xFFFFFF00;
                }
                break;
            // TODO: Proper alignment handling
            case 0b10: // LDRH
                reg(reg_dest) = ReadHWord(address);
                break;
            case 0b11: // LDSH
                reg(reg_dest) = ReadHWord(address);
                if (reg(reg_dest) & 0x8000)
                {
                    reg(reg_dest) |= 0xFFFF0000;
                }
                break;
            }
            break;
        }
        case THUMB_9:
        {
            // THUMB.9 Load store with immediate offset
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            uint immediate_value = (instruction >> 6) & 0x1F;
            switch ((instruction >> 11) & 3)
            {
            case 0b00: // STR
                WriteWord(reg(reg_base) + (immediate_value << 2), reg(reg_dest));
                break;
            case 0b01: // LDR
                reg(reg_dest) = ReadWordRotated(reg(reg_base) + (immediate_value << 2));
                break;
            case 0b10: // STRB
                WriteByte(reg(reg_base) + immediate_value, reg(reg_dest));
                break;
            case 0b11: // LDRB
                reg(reg_dest) = ReadByte(reg(reg_base) + immediate_value);
                break;
            }
            break;
        }
        case THUMB_10:
        {
            // THUMB.10 Load/store halfword
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            uint immediate_value = (instruction >> 6) & 0x1F;
            if (instruction & (1 << 11))
            {
                // LDRH
                // TODO: Alignment handling
                reg(reg_dest) = ReadHWord(reg(reg_base) + (immediate_value << 1));
            }
            else
            {
                // STRH
                WriteHWord(reg(reg_base) + (immediate_value << 1), reg(reg_dest));
            }
            break;
        }
        case THUMB_11:
        {
            // THUMB.11 SP-relative load/store
            uint immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            if (instruction & (1 << 11))
            {
                // LDR
                reg(reg_dest) = ReadWordRotated(reg(13) + (immediate_value << 2));
            }
            else
            {
                // STR
                WriteWord(reg(13) + (immediate_value << 2), reg(reg_dest));
            }
            break;
        }
        case THUMB_12:
        {
            // THUMB.12 Load address
            uint immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            if (instruction & (1 << 11))
            {
                // SP
                reg(reg_dest) = reg(13) + (immediate_value << 2);
            }
            else
            {
                // PC
                reg(reg_dest) = (r15 & ~2) + (immediate_value << 2);
            }
            break;
        }
        case THUMB_13:
        {
            // THUMB.13 Add offset to stack pointer
            uint immediate_value = instruction & 0x7F;
            if (instruction & 0x80)
            {
                reg(13) -= immediate_value;
            }
            else
            {
                reg(13) += immediate_value;
            }
            break;
        }
        case THUMB_14:
        {
            // THUMB.14 push/pop registers
            if (instruction & (1 << 11))
            {
                // POP
                for (int i = 0; i <= 7; i++)
                {
                    if (instruction & (1 << i))
                    { 
                        reg(i) = ReadWord(reg(13));
                        reg(13) += 4;
                    }
                }
                // Restore r15 if neccessary
                if (instruction & (1 << 8))
                {
                    r15 = ReadWord(reg(13)) & ~1;
                    flush_pipe = true;
                    reg(13) += 4;
                }
            }
            else
            {
                // PUSH
                // Store r14 if neccessary
                if (instruction & (1 << 8))
                {
                    reg(13) -= 4;
                    WriteWord(reg(13), reg(14));
                }
                for (int i = 7; i >= 0; i--)
                {
                    if (instruction & (1 << i))
                    {
                        reg(13) -= 4;
                        WriteWord(reg(13), reg(i));
                    }
                }
            }
            break;
        }
        case THUMB_15:
        {
            // THUMB.15 Multiple load/store
            int reg_base = (instruction >> 8) & 7;
            if (instruction & (1 << 11))
            {
                // LDMIA
                for (int i = 0; i <= 7; i++)
                {
                    if (instruction & (1 << i))
                    {
                        reg(i) = ReadWord(reg(reg_base));
                        reg(reg_base) += 4;
                    }
                }
            }
            else
            {
                // STMIA
                for (int i = 0; i <= 7; i++)
                {
                    if (instruction & (1 << i))
                    {
                        WriteWord(reg(reg_base), reg(i));
                        reg(reg_base) += 4;
                    }
                }
            }
            break;
        }
        case THUMB_16:
        {
            // THUMB.16 Conditional branch
            uint signed_immediate = instruction & 0xFF;
            bool execute = false;

            // Evaluate the condition
            switch ((instruction >> 8) & 0xF)
            {
            case 0x0: execute = (cpsr & ZeroFlag) == ZeroFlag; break;
            case 0x1: execute = (cpsr & ZeroFlag) != ZeroFlag; break;
            case 0x2: execute = (cpsr & CarryFlag) == CarryFlag; break;
            case 0x3: execute = (cpsr & CarryFlag) != CarryFlag; break;
            case 0x4: execute = (cpsr & SignFlag) == SignFlag; break;
            case 0x5: execute = (cpsr & SignFlag) != SignFlag; break;
            case 0x6: execute = (cpsr & OverflowFlag) == OverflowFlag; break;
            case 0x7: execute = (cpsr & OverflowFlag) != OverflowFlag; break;
            case 0x8: execute = ((cpsr & CarryFlag) == CarryFlag) & ((cpsr & ZeroFlag) != ZeroFlag); break;
            case 0x9: execute = ((cpsr & CarryFlag) != CarryFlag) || ((cpsr & ZeroFlag) == ZeroFlag); break;
            case 0xA: execute = ((cpsr & SignFlag) == SignFlag) == ((cpsr & OverflowFlag) == OverflowFlag); break;
            case 0xB: execute = ((cpsr & SignFlag) == SignFlag) != ((cpsr & OverflowFlag) == OverflowFlag); break;
            case 0xC: execute = ((cpsr & ZeroFlag) != ZeroFlag) && (((cpsr & SignFlag) == SignFlag) == ((cpsr & OverflowFlag) == OverflowFlag)); break;
            case 0xD: execute = ((cpsr & ZeroFlag) == ZeroFlag) || (((cpsr & SignFlag) == SignFlag) != ((cpsr & OverflowFlag) == OverflowFlag)); break;
            }

            // Perform the branch if the condition is met
            if (execute)
            {
                // Sign-extend the immediate value if neccessary
                if (signed_immediate & 0x80)
                {
                    signed_immediate |= 0xFFFFFF00;
                }
                r15 += (signed_immediate << 1);
                flush_pipe = true;
            }
            break;
        }
        case THUMB_17:
            // THUMB.17 Software Interrupt
            // TODO: I-bit?
            reg(14) = r15 - 2;
            spsr_svc = cpsr;
            r15 = 8;
            cpsr &= ~Thumb;
            cpsr = (cpsr & ~0x1F) | SVC;
            RemapRegisters();
            flush_pipe = true;
            break;
        case THUMB_18:
        {
            // THUMB.18 Unconditional branch
            uint immediate_value = (instruction & 0x3FF) << 1;
            if (instruction & 0x400)
            {
                immediate_value |= 0xFFFFF800;
            }
            r15 += immediate_value;
            flush_pipe = true;
            break;
        }
        case THUMB_19:
        {
            uint immediate_value = instruction & 0x7FF;
            if (instruction & (1 << 11))
            {
                // BH
                uint temp_pc = r15 - 2;
                uint value = reg(14) + (immediate_value << 1);
                
                // This was mostly written by looking at shonumis code lol
                value &= 0x7FFFFF;
                r15 &= ~0x7FFFFF;
                r15 |= value & ~1;

                reg(14) = temp_pc | 1;
                flush_pipe = true;
            }
            else
            {
                // BL
                reg(14) = r15 + (immediate_value << 12);
            }
            break;
        }
        }
#ifdef CPU_LOG
        cin.get();
#endif
    }
}