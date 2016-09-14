///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#include "arm.h"


const int ARM_1 = 1;
const int ARM_2 = 2;
const int ARM_3 = 3;
const int ARM_4 = 4;
const int ARM_5 = 5;
const int ARM_6 = 6;
const int ARM_7 = 7;
const int ARM_8 = 8;
const int ARM_9 = 9;
const int ARM_10 = 10;
const int ARM_11 = 11;
const int ARM_12 = 12;
const int ARM_13 = 13;
const int ARM_14 = 14;
const int ARM_15 = 15;
const int ARM_16 = 16;

namespace NanoboyAdvance
{
    int ARM7::Decode(u32 instruction)
    {
        u32 opcode = instruction & 0x0FFFFFFF;

        switch (opcode >> 26)
        {
        case 0b00:
            if (opcode & (1 << 25))
            {
                // ARM.8 Data processing and PSR transfer ... immediate
                return ARM_8;
            }
            else if ((opcode & 0xFF00FF0) == 0x1200F10)
            {
                // ARM.3 Branch and exchange
                return ARM_3;
            }
            else if ((opcode & 0x10000F0) == 0x90)
            {
                // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
                return opcode & (1 << 23) ? ARM_2 : ARM_1;
            }
            else if ((opcode & 0x10000F0) == 0x1000090)
            {
                // ARM.4 Single data swap
                return ARM_4;
            }
            else if ((opcode & 0x4000F0) == 0xB0)
            {
                // ARM.5 Halfword data transfer, register offset
                return ARM_5;
            }
            else if ((opcode & 0x4000F0) == 0x4000B0)
            {
                // ARM.6 Halfword data transfer, immediate offset
                return ARM_6;
            }
            else if ((opcode & 0xD0) == 0xD0)
            {
                // ARM.7 Signed data transfer (byte/halfword)
                return ARM_7;
            }
            else
            {
                // ARM.8 Data processing and PSR transfer
                return ARM_8;
            }
            break;
        case 0b01:
            // ARM.9 Single data transfer, ARM.10 Undefined
            return (opcode & 0x2000010) == 0x2000010 ? ARM_10 : ARM_9;
        case 0b10:
            // ARM.11 Block data transfer, ARM.12 Branch
            return opcode & (1 << 25) ? ARM_12 : ARM_11;
        case 0b11:
            // TODO: Optimize with a switch?
            //if (opcode & (1 << 25))
            {
                if (opcode & (1 << 24))
                {
                    // ARM.16 Software interrupt
                    return ARM_16;
                }
                else
                {
                    // ARM.14 Coprocessor data operation, ARM.15 Coprocessor register transfer
                    return opcode & 0x10 ? ARM_15 : ARM_14;
                }
            }
            //else
            //{
                // ARM.13 Coprocessor data transfer
            //    return ARM_13;
            //}
            break;
        }

        return 0;
    }

    void ARM7::Execute(u32 instruction, int type)
    {
        int condition = instruction >> 28;

        cycles += 1;

        // Check if the instruction will be executed
        if (condition != 0xE)
        {
            switch (condition)
            {
            case 0x0: if (!(cpsr & ZeroFlag))     return; break;
            case 0x1: if (  cpsr & ZeroFlag)      return; break;
            case 0x2: if (!(cpsr & CarryFlag))    return; break;
            case 0x3: if (  cpsr & CarryFlag)     return; break;
            case 0x4: if (!(cpsr & SignFlag))     return; break;
            case 0x5: if (  cpsr & SignFlag)      return; break;
            case 0x6: if (!(cpsr & OverflowFlag)) return; break;
            case 0x7: if (  cpsr & OverflowFlag)  return; break;
            case 0x8: if (!(cpsr & CarryFlag) ||  (cpsr & ZeroFlag)) return; break;
            case 0x9: if ( (cpsr & CarryFlag) && !(cpsr & ZeroFlag)) return; break;
            case 0xA: if ((cpsr & SignFlag) != (cpsr & OverflowFlag)) return; break;
            case 0xB: if ((cpsr & SignFlag) == (cpsr & OverflowFlag)) return; break;
            case 0xC: if ((cpsr & ZeroFlag) || ((cpsr & SignFlag) != (cpsr & OverflowFlag))) return; break;
            case 0xD: if (!(cpsr & ZeroFlag) && ((cpsr & SignFlag) == (cpsr & OverflowFlag))) return; break;
            case 0xE: break;
            case 0xF: return;
            }
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
            bool set_flags = instruction & (1 << 20);
            bool accumulate = instruction & (1 << 21);

            // Multiply the two operand and store result
            reg(reg_dest) = reg(reg_operand1) * reg(reg_operand2);

            // When the accumulate bit is set another register
            // may be added to the result register
            if (accumulate)
                reg(reg_dest) += reg(reg_operand3);

            // Update flags
            if (set_flags)
            {
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
            }

            // Calculate instruction timing
            cycles += memory->SequentialAccess(r[15], Memory::ACCESS_WORD);
            return;
        }
        case ARM_2:
        {
            // ARM.2 Multiply (accumulate) long
            int reg_operand1 = instruction & 0xF;
            int reg_operand2 = (instruction >> 8) & 0xF;
            int reg_dest_low = (instruction >> 12) & 0xF;
            int reg_dest_high = (instruction >> 16) & 0xF;
            bool set_flags = instruction & (1 << 20);
            bool accumulate = instruction & (1 << 21);
            bool sign_extend = instruction & (1 << 22);
            s64 result;

            if (sign_extend)
            {
                s64 operand1 = reg(reg_operand1);
                s64 operand2 = reg(reg_operand2);

                // Sign-extend operands
                operand1 |= operand1 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;
                operand2 |= operand2 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;

                // Calculate result
                result = operand1 * operand2;
            }
            else
            {
                u64 uresult = (u64)reg(reg_operand1) * (u64)reg(reg_operand2);
                result = uresult;
            }

            // When the accumulate bit is set another long value
            // formed by the original value of the destination registers
            // will be added to the result.
            if (accumulate)
            {
                s64 value = reg(reg_dest_high);

                // x86-compatible way for (rHIGH << 32) | rLOW
                value <<= 16;
                value <<= 16;
                value |= reg(reg_dest_low);
                
                result += value;
            }

            // Split and store result into the destination registers
            reg(reg_dest_low) = result & 0xFFFFFFFF;
            reg(reg_dest_high) = result >> 32;

            // Update flags
            if (set_flags)
            {
                CalculateSign(reg(reg_dest_high));
                CalculateZero(result);
            }         
            
            // Calculate instruction timing
            cycles += memory->SequentialAccess(r[15], Memory::ACCESS_WORD);
            return;
        }
        case ARM_3:
        {
            // ARM.3 Branch and exchange
            int reg_address = instruction & 0xF;

            // Bit0 being set in the address indicates
            // that the destination instruction is in THUMB mode.
            if (reg(reg_address) & 1)
            {
                r[15] = reg(reg_address) & ~1;
                cpsr |= ThumbFlag;

                // Emulate pipeline refill cycles
                cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_HWORD) +
                          memory->SequentialAccess(r[15] + 2, Memory::ACCESS_HWORD);
            }
            else
            {
                r[15] = reg(reg_address) & ~3;

                // Emulate pipeline refill cycles
                cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                          memory->SequentialAccess(r[15] + 4, Memory::ACCESS_WORD);
            }

            // Flush pipeline
            pipe.flush = true;

            return;
        }
        case ARM_4:
        {
            // ARM.4 Single data swap
            int reg_source = instruction & 0xF;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool swap_byte = instruction & (1 << 22);
            u32 memory_value;

            #ifdef DEBUG
            ASSERT(reg_source == 15 || reg_dest == 15 || reg_base == 15, LOG_WARN, 
                   "Single Data Swap, thou shall not use r15, r15=0x%x", r[15]);
            #endif 

            if (swap_byte)
            {
                // Reads one byte at rBASE into rDEST and overwrites
                // the byte at rBASE with the LSB of rSOURCE. 
                memory_value = ReadByte(reg(reg_base));
                WriteByte(reg(reg_base), (u8)reg(reg_source));
                reg(reg_dest) = memory_value;

                // Calculate instruction timing
                cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(reg(reg_base), Memory::ACCESS_BYTE) +
                              memory->NonSequentialAccess(reg(reg_source), Memory::ACCESS_BYTE);
            }
            else
            {
                // Reads one word at rBASE into rDEST and overwrites
                // the word at rBASE with rSOURCE.
                memory_value = ReadWordRotated(reg(reg_base));
                WriteWord(reg(reg_base), reg(reg_source));
                reg(reg_dest) = memory_value;

                // Calculate instruction timing
                cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(reg(reg_base), Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(reg(reg_source), Memory::ACCESS_WORD);
            }
            return;
        }
        case ARM_5:
        case ARM_6:
        case ARM_7:
        {
            // ARM.5 Halfword data transfer, register offset
            // ARM.6 Halfword data transfer, immediate offset
            // ARM.7 Signed data transfer (byte/halfword)
            u32 offset;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool load = instruction & (1 << 20);
            bool write_back = instruction & (1 << 21);
            bool immediate = instruction & (1 << 22);
            bool add_to_base = instruction & (1 << 23);
            bool pre_indexed = instruction & (1 << 24);
            u32 address = reg(reg_base);

            #ifdef DEBUG
            // Instructions neither write back if base register is r15
            // nor should they have the write-back bit set when being post-indexed.
            ASSERT(reg_base == 15 && write_back, LOG_ERROR, 
                   "Halfword and Signed Data Transfer, writeback to r15, r15=0x%x", r[15]);
            ASSERT(write_back && !pre_indexed, LOG_ERROR, 
                   "Halfword and Signed Data Transfer, writeback and post-indexed, r15=0x%x", r[15]);

            // Load-bit must be set when signed transfer is selected
            ASSERT(type == ARM_7 && !load, LOG_ERROR, 
                   "Halfword and Signed Data Transfer, signed bit and store bit set, r15=0x%x", r[15]);
            #endif

            // Decode immediate/register offset
            if (immediate)
                offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
            else
                offset = reg(instruction & 0xF);

            // Handle pre-indexing
            if (pre_indexed)
            {
                if (add_to_base)
                    address += offset;
                else
                    address -= offset;
            }

            // Handle ARM.7 signed reads or ARM.5/6 reads and writes.
            if (type == ARM_7)
            {
                u32 value;
                bool halfword = instruction & (1 << 5);
                    
                // Read either a sign-extended byte or hword.
                if (halfword)
                {
                    value = ReadHWordSigned(address);

                    // Calculate instruction timing
                    cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                                  memory->NonSequentialAccess(address, Memory::ACCESS_HWORD);
                }
                else
                {
                    value = ReadByte(address);

                    // Sign-extend the read byte.
                    if (value & 0x80)
                        value |= 0xFFFFFF00;

                    // Calculate instruction timing
                    cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                                  memory->NonSequentialAccess(address, Memory::ACCESS_BYTE);
                }
                    
                // Write result to rDEST.
                reg(reg_dest) = value;
            }
            else if (load)
            {
                reg(reg_dest) = ReadHWord(address);

                // Calculate instruction timing
                cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(address, Memory::ACCESS_HWORD);
            }
            else
            {
                if (reg_dest == 15)
                    WriteHWord(address, r[15] + 4);
                else
                    WriteHWord(address, reg(reg_dest));

                // Calculate instruction timing
                cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                          memory->NonSequentialAccess(address, Memory::ACCESS_HWORD);
            }

            // When the instruction either is pre-indexed and has the write-back bit
            // or it's post-indexed we must writeback the calculated address
            if ((write_back || !pre_indexed) && reg_base != reg_dest)
            {
                if (!pre_indexed)
                {
                    if (add_to_base)
                        address += offset;
                    else
                        address -= offset;
                }
                reg(reg_base) = address;
            }
            return;
        }
        case ARM_8:
        {
            // ARM.8 Data processing and PSR transfer
            bool set_flags = instruction & (1 << 20);
            int opcode = (instruction >> 21) & 0xF;

            // Decide wether the instruction is data processing or psr transfer
            // Look at official ARM docs to understand this condition!
            if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011)
            {
                // PSR transfer
                bool immediate = instruction & (1 << 25);
                bool use_spsr = instruction & (1 << 22);
                bool to_status = instruction & (1 << 21);

                if (to_status)
                {
                    // Move register to status register.
                    u32 mask = 0;
                    u32 operand;

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

                        // Apply immediate ROR-shift.
                        operand = (imm >> ror) | (imm << (32 - ror));
                    }
                    else
                    {
                        operand = reg(instruction & 0xF);
                    }

                    // Finally write either to SPSR or CPSR.
                    if (use_spsr)
                    {
                        *pspsr = (*pspsr & ~mask) | (operand & mask);
                    }
                    else
                    {
                        SaveRegisters();
                        cpsr = (cpsr & ~mask) | (operand & mask);
                        LoadRegisters();
                    }
                }
                else
                {
                    // Move satus register to register.
                    int reg_dest = (instruction >> 12) & 0xF;
                    reg(reg_dest) = use_spsr ? *pspsr : cpsr;
                }

                // Calculate instruction timing
                cycles += memory->SequentialAccess(r[15], Memory::ACCESS_WORD);
            }
            else
            {
                // Data processing
                int reg_dest = (instruction >> 12) & 0xF;
                int reg_operand1 = (instruction >> 16) & 0xF;
                bool immediate = instruction & (1 << 25);
                bool carry = cpsr & CarryFlag;
                u32 operand1 = reg(reg_operand1);
                u32 operand2;

                // Instruction prefetch timing
                cycles += memory->SequentialAccess(r[15], Memory::ACCESS_WORD);

                // Operand 2 may be an immediate value or a shifted register.
                if (immediate)
                {
                    int immediate_value = instruction & 0xFF;
                    int amount = ((instruction >> 8) & 0xF) << 1;

                    // Apply hardcoded rotation
                    operand2 = (immediate_value >> amount) | (immediate_value << (32 - amount));

                    // Rotation also overwrites the carry flag
                    if (amount != 0)
                        carry = (immediate_value >> (amount - 1)) & 1;
                }
                else
                {
                    u32 amount;
                    int reg_operand2 = instruction & 0xF;
                    bool shift_immediate = (instruction & (1 << 4)) ? false : true;

                    // Get operand raw value
                    operand2 = reg(reg_operand2);

                    // Get amount of shift.
                    if (shift_immediate)
                    {
                        amount = (instruction >> 7) & 0x1F;
                    }
                    else
                    {
                        int reg_shift = (instruction >> 8) & 0xF;
                        amount = reg(reg_shift);

                        // Internal cycle
                        cycles++;

                        // Due to the internal shift cycle r15 will be 12 bytes ahead, not 8.
                        if (reg_operand1 == 15) operand1 += 4;
                        if (reg_operand2 == 15) operand2 += 4;
                    }

                    // Apply shift by "amount" bits to raw value.
                    switch ((instruction >> 5) & 3)
                    {
                    case 0b00:
                        LSL(operand2, amount, carry);
                        break;
                    case 0b01:
                        LSR(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b10:
                        ASR(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b11:
                        ROR(operand2, amount, carry, shift_immediate);
                        break;
                    }
                }

                // The combination rDEST=15 and S-bit=1 triggers a CPU mode switch
                // effectivly switchting back to the saved mode (CPSR = SPSR).
                if (reg_dest == 15 && set_flags)
                {
                    // Flags will no longer be updated.
                    set_flags = false;

                    // Switches back to saved mode.
                    SaveRegisters();
                    cpsr = *pspsr;
                    LoadRegisters();
                }

                // Perform the actual operation
                switch (opcode)
                {
                case 0b0000:
                {
                    // Bitwise AND (AND)
                    u32 result = operand1 & operand2;

                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0001:
                {
                    // Bitwise EXOR (EOR)
                    u32 result = operand1 ^ operand2;

                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0010:
                {
                    // Subtraction (SUB)
                    u32 result = operand1 - operand2;

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
                case 0b0011:
                {
                    // Reverse subtraction (RSB)
                    u32 result = operand2 - operand1;

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
                case 0b0100:
                {
                    // Addition (ADD)
                    u32 result = operand1 + operand2;

                    if (set_flags)
                    {
                        u64 result_long = (u64)operand1 + (u64)operand2;

                        AssertCarry(result_long & 0x100000000);
                        CalculateOverflowAdd(result, operand1, operand2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0101:
                {
                    // Addition with Carry
                    int carry2 = (cpsr >> 29) & 1;
                    u32 result = operand1 + operand2 + carry2;

                    if (set_flags)
                    {
                        u64 result_long = (u64)operand1 + (u64)operand2 + (u64)carry2;

                        AssertCarry(result_long & 0x100000000);
                        CalculateOverflowAdd(result, operand1, operand2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0110:
                {
                    // Subtraction with Carry
                    int carry2 = (cpsr >> 29) & 1;
                    u32 result = operand1 - operand2 + carry2 - 1;

                    if (set_flags)
                    {
                        AssertCarry(operand1 >= (operand2 + carry2 - 1));
                        CalculateOverflowSub(result, operand1, operand2);
                        CalculateSign(result);
                        CalculateZero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0111:
                {
                    // Reverse Substraction with Carry
                    int carry2 = (cpsr >> 29) & 1;
                    u32 result = operand2 - operand1 + carry2 - 1;

                    if (set_flags)
                    {
                        AssertCarry(operand2 >= (operand1 + carry2 - 1));
                        CalculateOverflowSub(result, operand2, operand1);
                        CalculateSign(result);
                        CalculateZero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b1000:
                {
                    // Bitwise AND flags only (TST)
                    u32 result = operand1 & operand2;

                    CalculateSign(result);
                    CalculateZero(result);
                    AssertCarry(carry);
                    break;
                }
                case 0b1001:
                {
                    // Bitwise EXOR flags only (TEQ)
                    u32 result = operand1 ^ operand2;

                    CalculateSign(result);
                    CalculateZero(result);
                    AssertCarry(carry);
                    break;
                }
                case 0b1010:
                {
                    // Subtraction flags only (CMP)
                    u32 result = operand1 - operand2;

                    AssertCarry(operand1 >= operand2);
                    CalculateOverflowSub(result, operand1, operand2);
                    CalculateSign(result);
                    CalculateZero(result);
                    break;
                }
                case 0b1011:
                {
                    // Addition flags only (CMN)
                    u32 result = operand1 + operand2;
                    u64 result_long = (u64)operand1 + (u64)operand2;

                    AssertCarry(result_long & 0x100000000);
                    CalculateOverflowAdd(result, operand1, operand2);
                    CalculateSign(result);
                    CalculateZero(result);
                    break;
                }
                case 0b1100:
                {
                    // Bitwise OR (ORR)
                    u32 result = operand1 | operand2;

                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b1101:
                {
                    // Move into register (MOV)
                    if (set_flags)
                    {
                        CalculateSign(operand2);
                        CalculateZero(operand2);
                        AssertCarry(carry);
                    }

                    reg(reg_dest) = operand2;
                    break;
                }
                case 0b1110:
                {
                    // Bit Clear (BIC)
                    u32 result = operand1 & ~operand2;

                    if (set_flags)
                    {
                        CalculateSign(result);
                        CalculateZero(result);
                        AssertCarry(carry);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b1111: 
                {
                    // Move into register negated (MVN)
                    u32 not_operand2 = ~operand2;

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

                // Clear pipeline if r15 updated
                if (reg_dest == 15)
                {
                    pipe.flush = true;

                    // Emulate pipeline flush timings
                    cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->SequentialAccess(r[15] + 4, Memory::ACCESS_WORD);
                }
            }
            return;
        }
        case ARM_9:
        {
            // ARM.9 Load/store register/unsigned byte (Single Data Transfer)
            // TODO: Force user mode when instruction is post-indexed
            //       and has writeback bit (in system mode only?)
            u32 offset;
            int reg_dest = (instruction >> 12) & 0xF;
            int reg_base = (instruction >> 16) & 0xF;
            bool load = instruction & (1 << 20);
            bool write_back = instruction & (1 << 21);
            bool transfer_byte = instruction & (1 << 22);
            bool add_to_base = instruction & (1 << 23);
            bool pre_indexed = instruction & (1 << 24);
            bool immediate = (instruction & (1 << 25)) == 0;
            u32 address = reg(reg_base);

            #ifdef DEBUG
            // Writeback may not be used when rBASE=15 or post-indexed is enabled.
            ASSERT(reg_base == 15 && write_back, LOG_WARN, 
                   "Single Data Transfer, writeback to r15, r15=0x%x", r[15]);
            ASSERT(write_back && !pre_indexed, LOG_WARN,
                   "Single Data Transfer, writeback and post-indexed, r15=0x%x", r[15]);
            #endif

            // Decode offset.
            if (immediate)
            {
                // Immediate offset
                offset = instruction & 0xFFF;
            }
            else
            {
                // Register offset shifted by immediate value.
                int reg_offset = instruction & 0xF;
                u32 amount = (instruction >> 7) & 0x1F;
                int shift = (instruction >> 5) & 3;
                bool carry;

                #ifdef DEBUG
                ASSERT(reg_offset == 15, LOG_WARN, "Single Data Transfer, r15 used as offset, r15=0x%x", r[15]);
                #endif

                offset = reg(reg_offset);

                // Apply shift
                switch (shift)
                {
                case 0b00:
                    LSL(offset, amount, carry);
                    break;
                case 0b01:
                    LSR(offset, amount, carry, true);
                    break;
                case 0b10:
                    ASR(offset, amount, carry, true);
                    break;
                case 0b11:
                    carry = cpsr & CarryFlag;
                    ROR(offset, amount, carry, true);
                    break;
                }
            }

            // Handle pre-indexing
            if (pre_indexed)
            {
                if (add_to_base)
                    address += offset;
                else
                    address -= offset;
            }

            // Actual memory access.
            if (load)
            {
                // Load byte or word.
                if (transfer_byte)
                {
                    reg(reg_dest) = ReadByte(address);

                    // Calculate instruction timing
                    cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                                  memory->NonSequentialAccess(address, Memory::ACCESS_BYTE);
                }
                else
                {
                    reg(reg_dest) = ReadWordRotated(address);

                    // Calculate instruction timing
                    cycles += 1 + memory->SequentialAccess(r[15], Memory::ACCESS_WORD) +
                                  memory->NonSequentialAccess(address, Memory::ACCESS_WORD);
                }

                // Writing to r15 causes a pipeline-flush.    
                if (reg_dest == 15)
                {
                    pipe.flush = true;

                    // Emulate pipeline refill timings
                    cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->SequentialAccess(r[15] + 4, Memory::ACCESS_WORD);
                }
            }
            else
            {
                u32 value = reg(reg_dest);

                // r15 is +12 ahead in this cycle.
                if (reg_dest == 15)
                    value += 4;
                
                // Write byte or word.
                if (transfer_byte)
                {
                    WriteByte(address, value & 0xFF);

                    // Calculate instruction timing
                    cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(address, Memory::ACCESS_BYTE);
                }
                else
                {
                    WriteWord(address, value);

                    // Calculate instruction timing
                    cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_WORD) +
                              memory->NonSequentialAccess(address, Memory::ACCESS_WORD);
                }
            }

            // Handle post-indexing and writeback
            if (reg_base != reg_dest)
            {
                if (!pre_indexed)
                {
                    if (add_to_base)
                        reg(reg_base) += offset;
                    else
                        reg(reg_base) -= offset;
                }
                else if (write_back)
                {
                    reg(reg_base) = address;
                }
            }
            return;
        }
        case ARM_10:
            // ARM.10 Undefined
            #ifdef DEBUG
            LOG(LOG_ERROR, "Undefined instruction (0x%x), r15=0x%x", instruction, r[15]);
            #endif
            return;
        case ARM_11:
        {
            // ARM.11 Block Data Transfer
            int register_list = instruction & 0xFFFF;
            int base_register = (instruction >> 16) & 0xF;
            bool load_instr = instruction & (1 << 20);
            bool write_back = instruction & (1 << 21);
            bool force_user_mode = instruction & (1 << 22);
            bool increment_base = instruction & (1 << 23);
            bool pre_indexed = instruction & (1 << 24);
            bool switched_mode = false;
            bool first_access = false;
            u32 address = r[base_register];
            u32 address_old = address;
            int first_register = 0;
            int register_count = 0;
            int old_mode = 0;

            #ifdef DEBUG
            // Base register must not be r15
            ASSERT(base_register == 15, LOG_ERROR, "ARM.11, rB=15, r15=0x%x", r[15]);
            #endif

            // Switch to User mode if neccessary
            if (force_user_mode && (!load_instr || !(register_list & (1 << 15))))
            {
                #ifdef DEBUG
                ASSERT(write_back, LOG_ERROR, "ARM.11, writeback and modeswitch, r15=0x%x", r[15]);
                #endif

                // Save current mode and enter user mode
                old_mode = cpsr & ModeField;
                SaveRegisters();
                cpsr = (cpsr & ~ModeField) | (u32)Mode::USR;
                LoadRegisters();

                // Mark that we switched to user mode
                switched_mode = true;
            }

            // Find the first register and count registers
            for (int i = 0; i < 16; i++)
            {
                if (register_list & (1 << i))
                {
                    first_register = i;
                    register_count++;

                    // Continue to count registers
                    for (int j = i + 1; j < 16; j++)
                    {
                        if (register_list & (1 << j))
                            register_count++;
                    }
                    break;
                }
            }

            // One non-sequential prefetch cycle
            cycles += memory->NonSequentialAccess(r[15], Memory::ACCESS_HWORD);

            if (increment_base)
            {
                for (int i = first_register; i < 16; i++)
                {
                    if (register_list & (1 << i))
                    {
                        if (pre_indexed)
                            address += 4;

                        if (load_instr)
                        {
                            if (i == base_register)
                                write_back = false;

                            r[i] = ReadWord(address);

                            if (i == 15)
                            {
                                if (force_user_mode)
                                {
                                    SaveRegisters();
                                    cpsr = *pspsr;
                                    LoadRegisters();
                                }
                                pipe.flush = true;
                            }
                        }
                        else
                        {
                            if (i == first_register && i == base_register)
                                WriteWord(address, address_old);
                            else
                                WriteWord(address, r[i]);
                        }

                        if (!pre_indexed)
                            address += 4;

                        if (write_back)
                            r[base_register] = address;
                    }
                }
            }
            else
            {
                for (int i = 15; i >= first_register; i--)
                {
                    if (register_list & (1 << i))
                    {
                        if (pre_indexed)
                            address -= 4;

                        if (load_instr)
                        {
                            if (i == base_register)
                                write_back = false;

                            r[i] = ReadWord(address);

                            if (i == 15)
                            {
                                if (force_user_mode)
                                {
                                    SaveRegisters();
                                    cpsr = *pspsr;
                                    LoadRegisters();
                                }
                                pipe.flush = true;
                            }
                        }
                        else
                        {
                            if (i == first_register && i == base_register)
                                WriteWord(address, address_old);
                            else
                                WriteWord(address, r[i]);
                        }

                        if (!pre_indexed)
                            address -= 4;

                        if (write_back)
                            r[base_register] = address;
                    }
                }
            }

            // Switch back to original mode.
            if (switched_mode)
            {
                SaveRegisters();
                cpsr = (cpsr & ~ModeField) | old_mode;
                LoadRegisters();
            }
            return;
        }
        case ARM_12:
        {
            // ARM.12 Branch
            bool link = instruction & (1 << 24);
            u32 offset = instruction & 0xFFFFFF;
            if (offset & 0x800000)
            {
                offset |= 0xFF000000;
            }
            if (link)
            {
                reg(14) = r[15] - 4;
            }
            r[15] += offset << 2;
            pipe.flush = true;
            return;
        }
        case ARM_16:
        {
            // ARM.16 Software interrupt
            u32 bios_call = ReadByte(r[15] - 6);

            // Log to the console that we're issuing an interrupt.
            #ifdef DEBUG
            LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (arm)", bios_call, r[0], r[1], r[2], r[3], reg(14), r[15]);
            #endif

            // Dispatch SWI, either HLE or BIOS.
            if (hle)
            {
                SWI(bios_call);
            }
            else
            {
                // Store return address in r14<svc>
                r14_svc = r[15] - 4;

                // Save program status and switch mode
                spsr_svc = cpsr;
                SaveRegisters();
                cpsr = (cpsr & ~ModeField) | (u32)Mode::SVC | IrqDisable;
                LoadRegisters();

                // Jump to exception vector
                r[15] = (u32)Exception::SoftwareInterrupt;
                pipe.flush = true;
            }
            return;
        }
        }
    }
}
