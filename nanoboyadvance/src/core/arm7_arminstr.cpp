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

#define ARM_ERR 0
#define ARM_1 1
#define ARM_2 2
#define ARM_3 3
#define ARM_4 4
#define ARM_5 5
#define ARM_6 6
#define ARM_7 7
#define ARM_8 8
#define ARM_9 9
#define ARM_10 10
#define ARM_11 11
#define ARM_12 12
#define ARM_13 13
#define ARM_14 14
#define ARM_15 15
#define ARM_16 16

namespace NanoboyAdvance
{
    int ARM7::ARMDecode(uint instruction)
    {
        uint opcode = instruction & 0x0FFFFFFF;
        int section = ARM_ERR;
        switch (opcode >> 26)
        {
        case 0b00:
            if (opcode & (1 << 25))
            {
                // ARM.8 Data processing and PSR transfer ... immediate
                section = ARM_8;
            }
            // TODO: Check if we really must check all that bits
            else if ((opcode & 0xFFFFFF0) == 0x12FFF10)
            {
                // ARM.3 Branch and exchange
                section = ARM_3;
            }
            else if ((opcode & 0x10000F0) == 0x90)
            {
                // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
                section = opcode & (1 << 23) ? ARM_2 : ARM_1;
            }
            else if ((opcode & 0x10000F0) == 0x1000090)
            {
                // ARM.4 Single data swap
                section = ARM_4;
            }
            else if ((opcode & 0x4000F0) == 0xB0)
            {
                // ARM.5 Halfword data transfer, register offset
                section = ARM_5;
            }
            else if ((opcode & 0x4000F0) == 0x4000B0)
            {
                // ARM.6 Halfword data transfer, immediate offset
                section = ARM_6;
            }
            else if ((opcode & 0xD0) == 0xD0)
            {
                // ARM.7 Signed data transfer (byte/halfword)
                section = ARM_7;
            }
            else
            {
                // ARM.8 Data processing and PSR transfer
                section = ARM_8;
            }
            break;
        case 0b01:
            // ARM.9 Single data transfer, ARM.10 Undefined
            section = (opcode & 0x2000010) == 0x2000010 ? ARM_10 : ARM_9;
            break;
        case 0b10:
            // ARM.11 Block data transfer, ARM.12 Branch
            section = opcode & (1 << 25) ? ARM_12 : ARM_11;
            break;
        case 0b11:
            // TODO: Optimize with a switch?
            if (opcode & (1 << 25))
            {
                if (opcode & (1 << 24))
                {
                    // ARM.16 Software interrupt
                    section = ARM_16;
                }
                else
                {
                    // ARM.14 Coprocessor data operation, ARM.15 Coprocessor register transfer
                    section = opcode & 0x10 ? ARM_15 : ARM_14;
                }
            }
            else
            {
                // ARM.13 Coprocessor data transfer
                section = ARM_13;
            }
            break;
        }
        return section;
    }

    void ARM7::ARMExecute(uint instruction, int type)
    {
        int condition = instruction >> 28;
        bool execute = false;

#ifdef CPU_LOG
        // Log our status for debug reasons
        LOG(LOG_INFO, "Executing %s, r15=0x%x", ARMDisasm(r15 - 8, instruction).c_str(), r15);
        for (int i = 0; i < 16; i++)
        {
            if (i == 15)
            {
                cout << "r" << i << " = 0x" << std::hex << reg(i) << " (0x" << (reg(i) - 8) << ")" << std::dec << endl;
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

        // Check if the instruction will be executed
        switch (condition)
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
        case 0xE: execute = true; break;
        case 0xF: execute = false; break;
        }

        // If it will not be executed return now
        if (!execute)
        {
            return;
        }

        // Perform the actual execution
        switch (type)
        {
        case ARM_1:
        {
            // ARM.1 Multiply (accumulate)
            int reg_operand1 = instruction & 0xF;
            int reg_operand2 = (instruction >> 8) & 0xF;
            int reg_operand3 = (instruction >> 12) & 0xF;
            int reg_dest = (instruction >> 16) & 0xF;
            bool set_flags = (instruction & (1 << 20)) == (1 << 20);
            bool accumulate = (instruction & (1 << 21)) == (1 << 21);
            reg(reg_dest) = reg(reg_operand1) * reg(reg_operand2);
            if (accumulate)
            {
                reg(reg_dest) += reg(reg_operand3);
            }
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        }
        case ARM_2:
        {
            // ARM.2 Multiply (accumulate) long
            int reg_operand1 = instruction & 0xF;
            int reg_operand2 = (instruction >> 8) & 0xF;
            int reg_dest_low = (instruction >> 12) & 0xF;
            int reg_dest_high = (instruction >> 16) & 0xF;
            bool set_flags = (instruction & (1 << 20)) == (1 << 20);
            bool accumulate = (instruction & (1 << 21)) == (1 << 21);
            bool sign_extend = (instruction & (1 << 22)) == (1 << 22);
            slong result;
            if (sign_extend)
            {
                slong operand1 = reg(reg_operand1);
                slong operand2 = reg(reg_operand2);
                operand1 |= operand1 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;
                operand2 |= operand2 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;
                result = operand1 * operand2;
            }
            else
            {
                ulong uresult = (ulong)reg(reg_operand1) * (ulong)reg(reg_operand2);
                result = uresult;
            }
            if (accumulate)
            {
                slong value = reg(reg_dest_high);
                value <<= 16;
                value <<= 16;
                value |= reg(reg_dest_low);
                result += value;
            }
            reg(reg_dest_low) = result & 0xFFFFFFFF;
            reg(reg_dest_high) = result >> 32;
            CalculateSign(reg(reg_dest_high));
            CalculateZero(result);
            break;
        }
        case ARM_3:
        {
            // ARM.3 Branch and exchange
            int reg_address = instruction & 0xF;
            if (reg(reg_address) & 1)
            {
                r15 = reg(reg_address) & ~1;
                cpsr |= Thumb;
            }
            else
            {
                r15 = reg(reg_address) & ~3;
            }
            flush_pipe = true;
            break;
        }
        case ARM_4:
        {
            // ARM.4 Single data swap
            int reg_source = instruction & 0xF;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool swap_byte = (instruction & (1 << 22)) == (1 << 22);
            uint memory_value;
            ASSERT(reg_source == 15 || reg_dest == 15 || reg_base == 15, LOG_WARN, "Single Data Swap, thou shall not use r15, r15=0x%x", r15);
            if (swap_byte)
            {
                memory_value = ReadByte(reg(reg_base)) & 0xFF;
                WriteByte(reg(reg_base), reg(reg_source) & 0xFF);
                reg(reg_dest) = memory_value;
            }
            else
            {
                memory_value = ReadWordRotated(reg(reg_base));
                WriteWord(reg(reg_base), reg(reg_source));
                reg(reg_dest) = memory_value;
            }
            break;
        }
        // TODO: Recheck for correctness and look for possabilities to optimize this bunch of code
        case ARM_5:
        case ARM_6:
        case ARM_7:
        {
            // ARM.5 Halfword data transfer, register offset
            // ARM.6 Halfword data transfer, immediate offset
            // ARM.7 Signed data transfer (byte/halfword)
            // TODO: Proper alignment handling
            uint offset;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool load = (instruction & (1 << 20)) == (1 << 20);
            bool write_back = (instruction & (1 << 21)) == (1 << 21);
            bool immediate = (instruction & (1 << 22)) == (1 << 22);
            bool add_to_base = (instruction & (1 << 23)) == (1 << 23);
            bool pre_indexed = (instruction & (1 << 24)) == (1 << 24);
            uint address = reg(reg_base);

            // Instructions neither write back if base register is r15 nor should they have the write-back bit set when being post-indexed (post-indexing automatically writes back the address)
            ASSERT(reg_base == 15 && write_back, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not writeback to r15, r15=0x%x", r15);
            ASSERT(write_back && !pre_indexed, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not have write-back bit if being post-indexed, r15=0x%x", r15);

            // Load-bit shall not be unset when signed transfer is selected
            ASSERT(type == ARM_7 && !load, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not store in signed mode, r15=0x%x", r15);

            // If the instruction is immediate take an 8-bit immediate value as offset, otherwise take the contents of a register as offset
            if (immediate)
            {
                offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
            }
            else
            {
                int reg_offset = instruction & 0xF;
                ASSERT(reg_offset == 15, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not take r15 as offset, r15=0x%x", r15);
                offset = reg(reg_offset);
            }

            // If the instruction is pre-indexed we must add/subtract the offset beforehand
            if (pre_indexed)
            {
                if (add_to_base)
                {
                    address += offset;
                }
                else
                {
                    address -= offset;
                }
            }

            // Perform the actual load / store operation
            if (load)
            {
                // TODO: Check if pipeline is flushed when reg_dest is r15
                if (type == ARM_7)
                {
                    bool halfword = (instruction & (1 << 5)) == (1 << 5);
                    uint value;
                    if (halfword)
                    {
                        value = ReadHWord(address);
                        if (value & 0x8000)
                        {
                            value |= 0xFFFF0000;
                        }
                    }
                    else
                    {
                        value = ReadByte(address);
                        if (value & 0x80)
                        {
                            value |= 0xFFFFFF00;
                        }
                    }
                    reg(reg_dest) = value;
                }
                else
                {
                    reg(reg_dest) = ReadHWord(address);
                }
            }
            else
            {
                if (reg_dest == 15)
                {
                    WriteHWord(address, r15 + 4);
                }
                else
                {
                    WriteHWord(address, reg(reg_dest));
                }
            }

            // When the instruction either is pre-indexed and has the write-back bit or it's post-indexed we must writeback the calculated address
            if ((write_back || !pre_indexed) && reg_base != reg_dest)
            {
                if (!pre_indexed)
                {
                    if (add_to_base)
                    {
                        address += offset;
                    }
                    else
                    {
                        address -= offset;
                    }
                }
                reg(reg_base) = address;
            }
            break;
        }
        case ARM_8:
        {
            // ARM.8 Data processing and PSR transfer
            bool set_flags = (instruction & (1 << 20)) == (1 << 20);
            int opcode = (instruction >> 21) & 0xF;

            // Determine wether the instruction is data processing or psr transfer
            if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011)
            {
                // PSR transfer
                bool immediate = instruction & (1 << 25) ? true : false;
                bool use_spsr = instruction & (1 << 22) ? true : false;
                bool msr = instruction & (1 << 21) ? true : false;

                if (msr)
                {
                    // MSR
                    uint mask = 0;
                    uint operand;

                    // Depending of the fsxc bits some bits are overwritten or not
                    if (instruction & (1 << 16)) mask |= 0x000000FF;
                    if (instruction & (1 << 17)) mask |= 0x0000FF00;
                    if (instruction & (1 << 18)) mask |= 0x00FF0000;
                    if (instruction & (1 << 19)) mask |= 0xFF000000;

                    // Decode the value written to cpsr/spsr
                    if (immediate)
                    {
                        int imm = instruction & 0xFF;
                        int ror = ((instruction >> 8) & 0xF) << 1;
                        operand = (imm >> ror) | (imm << (32 - ror));
                    }
                    else
                    {
                        int reg_source = instruction & 0xF;
                        operand = reg(reg_source);
                    }

                    // Write
                    if (use_spsr)
                    {
                        *pspsr = (*pspsr & ~mask) | (operand & mask);
                    }
                    else
                    {
                        cpsr = (cpsr & ~mask) | (operand & mask);
                        RemapRegisters();
                    }
                }
                else
                {
                    // MRS
                    int reg_dest = (instruction >> 12) & 0xF;
                    reg(reg_dest) = use_spsr ? *pspsr : cpsr;
                }
            }
            else
            {
                // Data processing
                int reg_dest = (instruction >> 12) & 0xF;
                int reg_operand1 = (instruction >> 16) & 0xF;
                bool immediate = (instruction & (1 << 25)) == (1 << 25);
                uint operand1 = reg(reg_operand1);
                uint operand2;
                bool carry = (cpsr & CarryFlag) == CarryFlag;

                // Operand 2 can either be an 8 bit immediate value rotated right by 4 bit value or the value of a register shifted by a specific amount
                if (immediate)
                {
                    int immediate_value = instruction & 0xFF;
                    int amount = ((instruction >> 8) & 0xF) << 1;
                    operand2 = (immediate_value >> amount) | (immediate_value << (32 - amount));
                    if (amount != 0)
                    {
                        carry = (immediate_value >> (amount - 1)) & 1 ? true : false;
                    }
                }
                else
                {
                    bool shift_immediate = (instruction & (1 << 4)) ? false : true;
                    int reg_operand2 = instruction & 0xF;
                    uint amount;
                    operand2 = reg(reg_operand2);

                    // The amount is either the value of a register or a 5 bit immediate
                    if (shift_immediate)
                    {
                        amount = (instruction >> 7) & 0x1F;
                    }
                    else
                    {
                        int reg_shift = (instruction >> 8) & 0xF;
                        amount = reg(reg_shift);

                        // When using a register to specify the shift amount r15 will be 12 bytes ahead instead of 8 bytes
                        if (reg_operand1 == 15)
                        {
                            operand1 += 4;
                        }
                        if (reg_operand2 == 15)
                        {
                            operand2 += 4;
                        }
                    }

                    // Perform the actual shift/rotate
                    switch ((instruction >> 5) & 3)
                    {
                    case 0b00:
                        // Logical Shift Left
                        LSL(operand2, amount, carry);
                        break;
                    case 0b01:
                        // Logical Shift Right
                        LSR(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b10:
                    {
                        // Arithmetic Shift Right
                        ASR(operand2, amount, carry, shift_immediate);
                        break;
                    }
                    case 0b11:
                        // Rotate Right
                        ROR(operand2, amount, carry, shift_immediate);
                        break;
                    }
                }

                // When destination register is r15 and s bit is set rather than updating the flags restore cpsr
                // This is allows for restoring r15 and cpsr at the same time
                if (reg_dest == 15 && set_flags)
                {
                    set_flags = false;
                    cpsr = *pspsr;
                    RemapRegisters();
                }

                // Perform the actual operation
                switch (opcode)
                {
                case 0b0000: // AND
                {
                    uint result = operand1 & operand2;
                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0001: // EOR
                {
                    uint result = operand1 ^ operand2;
                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0010: // SUB
                {
                    uint result = operand1 - operand2;
                    if (set_flags)
                    {
                        AssertCarry(operand1 >= operand2);
                        CalculateOverflowSub(result, operand1, operand2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0011: // RSB
                {
                    uint result = operand2 - operand1;
                    if (set_flags)
                    {
                        AssertCarry(operand2 >= operand1);
                        CalculateOverflowSub(result, operand2, operand1);
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0100: // ADD
                {
                    uint result = operand1 + operand2;
                    if (set_flags)
                    {
                        ulong result_long = (ulong)operand1 + (ulong)operand2;
                        AssertCarry((result_long & 0x100000000) ? true : false);
                        CalculateOverflowAdd(result, operand1, operand2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0101: // ADC
                {
                    int carry2 = (cpsr >> 29) & 1;
                    uint result = operand1 + operand2 + carry2;
                    if (set_flags)
                    {
                        ulong result_long = (ulong)operand1 + (ulong)operand2 + (ulong)carry2;
                        AssertCarry((result_long & 0x100000000) ? true : false);
                        CalculateOverflowAdd(result, operand1, operand2 + carry2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0110: // SBC
                {
                    int carry2 = (cpsr >> 29) & 1;
                    uint result = operand1 - operand2 + carry2 - 1;
                    if (set_flags)
                    {
                        AssertCarry(operand1 >= (operand2 + carry2 - 1));
                        CalculateOverflowSub(result, operand1, (operand2 + carry2 - 1));
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b0111: // RSC
                {
                    int carry2 = (cpsr >> 29) & 1;
                    uint result = operand2 - operand1 + carry2 - 1;
                    if (set_flags)
                    {
                        AssertCarry(operand2 >= (operand1 + carry2 - 1));
                        CalculateOverflowSub(result, operand2, (operand1 + carry2 - 1));
                        CalculateSign(result);
                        CalculateZero(result);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b1000: // TST
                {
                    uint result = operand1 & operand2;
                    CalculateSign(result);
                    CalculateZero(result);
                    AssertCarry(carry);
                    break;
                }
                case 0b1001: // TEQ
                {
                    uint result = operand1 ^ operand2;
                    CalculateSign(result);
                    CalculateZero(result);
                    AssertCarry(carry);
                    break;
                }
                case 0b1010: // CMP
                {
                    uint result = operand1 - operand2;
                    AssertCarry(operand1 >= operand2);
                    CalculateOverflowSub(result, operand1, operand2);
                    CalculateSign(result);
                    CalculateZero(result);
                    break;
                }
                case 0b1011: // CMN
                {
                    uint result = operand1 + operand2;
                    ulong result_long = (ulong)operand1 + (ulong)operand2;
                    AssertCarry((result_long & 0x100000000) ? true : false);
                    CalculateOverflowAdd(result, operand1, operand2);
                    CalculateSign(result);
                    CalculateZero(result);
                    break;
                }
                case 0b1100: // ORR
                {
                    uint result = operand1 | operand2;
                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b1101: // MOV
                {
                    if (set_flags)
                    {
                        CalculateSign(operand2);
                        CalculateZero(operand2);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = operand2;
                    break;
                }
                case 0b1110: // BIC
                {
                    uint result = operand1 & ~operand2;
                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = result;
                    break;
                }
                case 0b1111: // MVN
                {
                    uint not_operand2 = ~operand2;
                    if (set_flags)
                    {
                        CalculateSign(not_operand2);
                        CalculateZero(not_operand2);
                        AssertCarry(carry);
                    }
                    reg(reg_dest) = not_operand2;
                    break;
                }
                }

                // When writing to r15 initiate pipeline flush
                if (reg_dest == 15)
                {
                    flush_pipe = true;
                }
            }
            break;
        }
        case ARM_9:
        {
            // ARM.9 Load/store register/unsigned byte (Single Data Transfer)
            // TODO: Force user mode when instruction is post-indexed and has writeback bit (in system mode only?)
            uint offset;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool load = (instruction & (1 << 20)) == (1 << 20);
            bool write_back = (instruction & (1 << 21)) == (1 << 21);
            bool transfer_byte = (instruction & (1 << 22)) == (1 << 22);
            bool add_to_base = (instruction & (1 << 23)) == (1 << 23);
            bool pre_indexed = (instruction & (1 << 24)) == (1 << 24);
            bool immediate = (instruction & (1 << 25)) == 0;
            uint address = reg(reg_base);

            // Instructions neither write back if base register is r15 nor should they have the write-back bit set when being post-indexed (post-indexing automatically writes back the address)
            ASSERT(reg_base == 15 && write_back, LOG_WARN, "Single Data Transfer, thou shall not writeback to r15, r15=0x%x", r15);
            ASSERT(write_back && !pre_indexed, LOG_WARN, "Single Data Transfer, thou shall not have write-back bit if being post-indexed, r15=0x%x", r15);

            // The offset added to the base address can either be an 12 bit immediate value or a register shifted by 5 bit immediate value
            if (immediate)
            {
                offset = instruction & 0xFFF;
            }
            else
            {
                int reg_offset = instruction & 0xF;
                uint amount = (instruction >> 7) & 0x1F;
                int shift = (instruction >> 5) & 3;
                bool carry; // this is not actually used but we need a var for carry out.

                ASSERT(reg_offset == 15, LOG_WARN, "Single Data Transfer, thou shall not use r15 as offset, r15=0x%x", r15);

                offset = reg(reg_offset);

                // Perform the actual shift
                switch (shift)
                {
                case 0b00:
                {
                    // Logical Shift Left
                    LSL(offset, amount, carry);
                    break;
                }
                case 0b01:
                {
                    // Logical Shift Right
                    LSR(offset, amount, carry, true);
                    break;
                }
                case 0b10:
                {
                    // Arithmetic Shift Right
                    ASR(offset, amount, carry, true);
                    break;
                }
                case 0b11:
                {
                    // Rotate Right
                    carry = (cpsr & CarryFlag) ? true : false;
                    ROR(offset, amount, carry, true);
                    break;
                }
                }
            }

            // If the instruction is pre-indexed we must add/subtract the offset beforehand
            if (pre_indexed)
            {
                if (add_to_base)
                {
                    address += offset;
                }
                else
                {
                    address -= offset;
                }
            }

            // Perform the actual load / store operation
            if (load)
            {
                if (transfer_byte)
                {
                    reg(reg_dest) = ReadByte(address);
                }
                else
                {
                    reg(reg_dest) = ReadWordRotated(address);
                }
                if (reg_dest == 15)
                {
                    flush_pipe = true;
                }
            }
            else
            {
                uint value = reg(reg_dest);
                if (reg_dest == 15)
                {
                    value += 4;
                }
                if (transfer_byte)
                {
                    WriteByte(address, value & 0xFF);
                }
                else
                {
                    WriteWord(address, value);
                }
            }

            // When the instruction either is pre-indexed and has the write-back bit or it's post-indexed we must writeback the calculated address
            if (reg_base != reg_dest)
            {
                if (!pre_indexed)
                {
                    if (add_to_base)
                    {
                        reg(reg_base) += offset;
                    }
                    else
                    {
                        reg(reg_base) -= offset;
                    }
                }
                else if (write_back)
                {
                    reg(reg_base) = address;
                }
            }
            break;
        }
        case ARM_10:
            // ARM.10 Undefined
            LOG(LOG_ERROR, "Undefined instruction (0x%x), r15=0x%x", instruction, r15);
            break;
        case ARM_11:
        {
            // ARM.11 Block Data Transfer
            // TODO: Handle empty register list
            //       Correct transfer order for stm (this is needed for some io transfers)
            //       See gbatek for both
            bool pc_in_list = (instruction & (1 << 15)) == (1 << 15);
            int reg_base = (instruction >> 16) & 0xF;
            bool load = (instruction & (1 << 20)) == (1 << 20);
            bool write_back = (instruction & (1 << 21)) == (1 << 21);
            bool s_bit = (instruction & (1 << 22)) == (1 << 22); // TODO: Give this a meaningful name
            bool add_to_base = (instruction & (1 << 23)) == (1 << 23);
            bool pre_indexed = (instruction & (1 << 24)) == (1 << 24);
            uint address = reg(reg_base);
            uint old_address = address;
            bool switched_mode = false;
            int old_mode;
            int first_register = 0;

            // Base register must not be r15
            ASSERT(reg_base == 15, LOG_WARN, "Block Data Tranfser, thou shall not take r15 as base register, r15=0x%x", r15);

            // If the s bit is set and the instruction is either a store or r15 is not in the list switch to user mode
            if (s_bit && (!load || !pc_in_list))
            {
                // Writeback must not be activated in this case
                ASSERT(write_back, LOG_WARN, "Block Data Transfer, thou shall not do user bank transfer with writeback, r15=0x%x", r15);

                // Save current mode and enter user mode
                old_mode = cpsr & 0x1F;
                cpsr = (cpsr & ~0x1F) | User;
                RemapRegisters();

                // Mark that we switched to user mode
                switched_mode = true;
            }

            // Find the first register
            for (int i = 0; i < 16; i++)
            {
                if (instruction & (1 << i))
                {
                    first_register = i;
                    break;
                }
            }

            // Walk through the register list
            // TODO: Start with the first register (?)
            //       Remove code redundancy
            if (add_to_base)
            {
                for (int i = 0; i < 16; i++)
                {
                    // Determine if the current register will be loaded/saved
                    if (instruction & (1 << i))
                    {
                        // If instruction is pre-indexed we must update address beforehand
                        if (pre_indexed)
                        {
                            address += 4;
                        }

                        // Perform the actual load / store operation
                        if (load)
                        {
                            // Overwriting the base disables writeback
                            if (i == reg_base)
                            {
                                write_back = false;
                            }

                            // Load the register
                            reg(i) = ReadWord(address);

                            // If r15 is overwritten, the pipeline must be flushed
                            if (i == 15)
                            {
                                // If the s bit is set a mode switch is performed
                                if (s_bit)
                                {
                                    // spsr_<mode> must not be copied to cpsr in user mode because user mode has not such a register
                                    ASSERT((cpsr & 0x1F) == User, LOG_ERROR, "Block Data Transfer is about to copy spsr_<mode> to cpsr, however we are in user mode, r15=0x%x", r15);

                                    cpsr = *pspsr;
                                    RemapRegisters();
                                }
                                flush_pipe = true;
                            }
                        }
                        else
                        {
                            // When the base register is the first register in the list its original value is written
                            if (i == first_register && i == reg_base)
                            {
                                WriteWord(address, old_address);
                            }
                            else
                            {
                                WriteWord(address, reg(i));
                            }
                        }

                        // If instruction is not pre-indexed we must update address afterwards
                        if (!pre_indexed)
                        {
                            address += 4;
                        }

                        // If the writeback is specified the base register must be updated after each register
                        if (write_back)
                        {
                            reg(reg_base) = address;
                        }
                    }
                }
            }
            else
            {
                for (int i = 15; i >= 0; i--)
                {
                    // Determine if the current register will be loaded/saved
                    if (instruction & (1 << i))
                    {
                        // If instruction is pre-indexed we must update address beforehand
                        if (pre_indexed)
                        {
                            address -= 4;
                        }

                        // Perform the actual load / store operation
                        if (load)
                        {
                            // Overwriting the base disables writeback
                            if (i == reg_base)
                            {
                                write_back = false;
                            }

                            // Load the register
                            reg(i) = ReadWord(address);

                            // If r15 is overwritten, the pipeline must be flushed
                            if (i == 15)
                            {
                                // If the s bit is set a mode switch is performed
                                if (s_bit)
                                {
                                    // spsr_<mode> must not be copied to cpsr in user mode because user mode has no such a register
                                    ASSERT((cpsr & 0x1F) == User, LOG_ERROR, "Block Data Transfer is about to copy spsr_<mode> to cpsr, however we are in user mode, r15=0x%x", r15);

                                    cpsr = *pspsr;
                                    RemapRegisters();
                                }
                                flush_pipe = true;
                            }
                        }
                        else
                        {
                            // When the base register is the first register in the list its original value is written
                            if (i == first_register && i == reg_base)
                            {
                                WriteWord(address, old_address);
                            }
                            else
                            {
                                WriteWord(address, reg(i));
                            }
                        }

                        // If instruction is not pre-indexed we must update address afterwards
                        if (!pre_indexed)
                        {
                            address -= 4;
                        }

                        // If the writeback is specified the base register must be updated after each register
                        if (write_back)
                        {
                            reg(reg_base) = address;
                        }
                    }
                }
            }

            // If we switched mode it's time now to restore the previous mode
            if (switched_mode)
            {
                cpsr = (cpsr & ~0x1F) | old_mode;
                RemapRegisters();
            }
            break;
        }
        case ARM_12:
        {
            // ARM.12 Branch
            bool link = (instruction & (1 << 24)) == (1 << 24);
            uint offset = instruction & 0xFFFFFF;
            if (offset & 0x800000)
            {
                offset |= 0xFF000000;
            }
            if (link)
            {
                reg(14) = r15 - 4;
            }
            r15 += offset << 2;
            flush_pipe = true;
            break;
        }
        case ARM_13:
            // ARM.13 Coprocessor data transfer
            LOG(LOG_ERROR, "Unimplemented coprocessor data transfer, r15=0x%x", r15);
            break;
        case ARM_14:
            // ARM.14 Coprocessor data operation
            LOG(LOG_ERROR, "Unimplemented coprocessor data operation, r15=0x%x", r15);
            break;
        case ARM_15:
            // ARM.15 Coprocessor register transfer
            LOG(LOG_ERROR, "Unimplemented coprocessor register transfer, r15=0x%x", r15);
            break;
        // TODO: Check wether swi is still executed when IRQ is disabled
        //       Implement HLE version
        case ARM_16:
            // ARM.16 Software interrupt
            if ((cpsr & IRQDisable) == 0)
            {
                //LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (arm)", ReadByte(r15 - 6), r0, r1, r2, r3, reg(14), r15);
                uint bios_call = ReadByte(r15 - 6);

                // See if we must trigger a breakpoint (and do it)
                TriggerSVCBreakpoint(bios_call);

                // Actual emulation
                if (hle)
                {
                    SWI(ReadByte(r15 - 6));
                }
                else
                {
                    r14_svc = r15 - 4;
                    r15 = 0x8;
                    flush_pipe = true;
                    spsr_svc = cpsr;
                    cpsr = (cpsr & ~0x1F) | SVC | IRQDisable;
                    RemapRegisters();
                }
            }
            break;
        }
#ifdef CPU_LOG
        cin.get();
#endif
    }
}
