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


#include "arm.hpp"


// Artifact of old code. Remove when I got time.
#define reg(i) this->m_state.m_reg[i]

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

namespace GBA
{
    int arm::Decode(u32 instruction)
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

    void arm::Execute(u32 instruction, int type)
    {
        int condition = instruction >> 28;

        cycles += 1;

        // Check if the instruction will be executed
        if (condition != 0xE)
        {
            switch (condition)
            {
            case 0x0: if (!(m_state.m_cpsr & MASK_ZFLAG))     return; break;
            case 0x1: if (  m_state.m_cpsr & MASK_ZFLAG)      return; break;
            case 0x2: if (!(m_state.m_cpsr & MASK_CFLAG))    return; break;
            case 0x3: if (  m_state.m_cpsr & MASK_CFLAG)     return; break;
            case 0x4: if (!(m_state.m_cpsr & MASK_NFLAG))     return; break;
            case 0x5: if (  m_state.m_cpsr & MASK_NFLAG)      return; break;
            case 0x6: if (!(m_state.m_cpsr & MASK_VFLAG)) return; break;
            case 0x7: if (  m_state.m_cpsr & MASK_VFLAG)  return; break;
            case 0x8: if (!(m_state.m_cpsr & MASK_CFLAG) ||  (m_state.m_cpsr & MASK_ZFLAG)) return; break;
            case 0x9: if ( (m_state.m_cpsr & MASK_CFLAG) && !(m_state.m_cpsr & MASK_ZFLAG)) return; break;
            case 0xA: if ((m_state.m_cpsr & MASK_NFLAG) != (m_state.m_cpsr & MASK_VFLAG)) return; break;
            case 0xB: if ((m_state.m_cpsr & MASK_NFLAG) == (m_state.m_cpsr & MASK_VFLAG)) return; break;
            case 0xC: if ((m_state.m_cpsr & MASK_ZFLAG) || ((m_state.m_cpsr & MASK_NFLAG) != (m_state.m_cpsr & MASK_VFLAG))) return; break;
            case 0xD: if (!(m_state.m_cpsr & MASK_ZFLAG) && ((m_state.m_cpsr & MASK_NFLAG) == (m_state.m_cpsr & MASK_VFLAG))) return; break;
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
                update_sign(reg(reg_dest));
                update_zero(reg(reg_dest));
            }

            // Calculate instruction timing
            cycles += Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD);
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
                update_sign(reg(reg_dest_high));
                update_zero(result);
            }

            // Calculate instruction timing
            cycles += Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD);
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
                m_state.m_reg[15] = reg(reg_address) & ~1;
                m_state.m_cpsr |= MASK_THUMB;

                // Emulate pipeline refill cycles
                cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_HWORD) +
                          Memory::SequentialAccess(m_state.m_reg[15] + 2, ACCESS_HWORD);
            }
            else
            {
                m_state.m_reg[15] = reg(reg_address) & ~3;

                // Emulate pipeline refill cycles
                cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                          Memory::SequentialAccess(m_state.m_reg[15] + 4, ACCESS_WORD);
            }

            // Flush pipeline
            m_Pipe.m_Flush = true;

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
                   "Single Data Swap, thou shall not use r15, r15=0x%x", m_state.m_reg[15]);
            #endif

            if (swap_byte)
            {
                // Reads one byte at rBASE into rDEST and overwrites
                // the byte at rBASE with the LSB of rSOURCE.
                memory_value = ReadByte(reg(reg_base));
                WriteByte(reg(reg_base), (u8)reg(reg_source));
                reg(reg_dest) = memory_value;

                // Calculate instruction timing
                cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::NonSequentialAccess(reg(reg_base), ACCESS_BYTE) +
                              Memory::NonSequentialAccess(reg(reg_source), ACCESS_BYTE);
            }
            else
            {
                // Reads one word at rBASE into rDEST and overwrites
                // the word at rBASE with rSOURCE.
                memory_value = ReadWordRotated(reg(reg_base));
                WriteWord(reg(reg_base), reg(reg_source));
                reg(reg_dest) = memory_value;

                // Calculate instruction timing
                cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::NonSequentialAccess(reg(reg_base), ACCESS_WORD) +
                              Memory::NonSequentialAccess(reg(reg_source), ACCESS_WORD);
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
            ASSERT(reg_base == 15 && write_back, LOG_ERrotate_right,
                   "Halfword and Signed Data Transfer, writeback to r15, r15=0x%x", m_state.m_reg[15]);
            ASSERT(write_back && !pre_indexed, LOG_ERrotate_right,
                   "Halfword and Signed Data Transfer, writeback and post-indexed, r15=0x%x", m_state.m_reg[15]);

            // Load-bit must be set when signed transfer is selected
            ASSERT(type == ARM_7 && !load, LOG_ERrotate_right,
                   "Halfword and Signed Data Transfer, signed bit and store bit set, r15=0x%x", m_state.m_reg[15]);
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
                    cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                                  Memory::NonSequentialAccess(address, ACCESS_HWORD);
                }
                else
                {
                    value = ReadByte(address);

                    // Sign-extend the read byte.
                    if (value & 0x80)
                        value |= 0xFFFFFF00;

                    // Calculate instruction timing
                    cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                                  Memory::NonSequentialAccess(address, ACCESS_BYTE);
                }

                // Write result to rDEST.
                reg(reg_dest) = value;
            }
            else if (load)
            {
                reg(reg_dest) = ReadHWord(address);

                // Calculate instruction timing
                cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::NonSequentialAccess(address, ACCESS_HWORD);
            }
            else
            {
                if (reg_dest == 15)
                    WriteHWord(address, m_state.m_reg[15] + 4);
                else
                    WriteHWord(address, reg(reg_dest));

                // Calculate instruction timing
                cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                          Memory::NonSequentialAccess(address, ACCESS_HWORD);
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

                    // Decode the value written to m_state.m_cpsr/spsr
                    if (immediate)
                    {
                        int imm = instruction & 0xFF;
                        int ror = ((instruction >> 8) & 0xF) << 1;

                        // Apply immediate rotate_right-shift.
                        operand = (imm >> ror) | (imm << (32 - ror));
                    }
                    else
                    {
                        operand = reg(instruction & 0xF);
                    }

                    // Finally write either to SPSR or m_state.m_cpsr.
                    if (use_spsr)
                    {
                        *m_state.m_spsr_ptr = (*m_state.m_spsr_ptr & ~mask) | (operand & mask);
                    }
                    else
                    {
                        ////SaveRegisters();
                        ////m_state.m_cpsr = (m_state.m_cpsr & ~mask) | (operand & mask);
                        ////LoadRegisters();
                        u32 value = operand & mask;

                        // todo: make sure that mode might actually be affected.
                        m_state.switch_mode(static_cast<cpu_mode>(value & MASK_MODE));
                        m_state.m_cpsr = (m_state.m_cpsr & ~mask) | value;
                    }
                }
                else
                {
                    // Move satus register to register.
                    int reg_dest = (instruction >> 12) & 0xF;
                    reg(reg_dest) = use_spsr ? *m_state.m_spsr_ptr : m_state.m_cpsr;
                }

                // Calculate instruction timing
                cycles += Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD);
            }
            else
            {
                // Data processing
                int reg_dest = (instruction >> 12) & 0xF;
                int reg_operand1 = (instruction >> 16) & 0xF;
                bool immediate = instruction & (1 << 25);
                bool carry = m_state.m_cpsr & MASK_CFLAG;
                u32 operand1 = reg(reg_operand1);
                u32 operand2;

                // Instruction prefetch timing
                cycles += Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD);

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
                        logical_shift_left(operand2, amount, carry);
                        break;
                    case 0b01:
                        logical_shift_right(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b10:
                        arithmetic_shift_right(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b11:
                        rotate_right(operand2, amount, carry, shift_immediate);
                        break;
                    }
                }

                // The combination rDEST=15 and S-bit=1 triggers a CPU mode switch
                // effectivly switchting back to the saved mode (m_state.m_cpsr = SPSR).
                if (reg_dest == 15 && set_flags)
                {
                    // Flags will no longer be updated.
                    set_flags = false;

                    // Switches back to saved mode.
                    ////SaveRegisters();
                    ////m_state.m_cpsr = *m_state.m_spsr_ptr;
                    ////LoadRegisters();
                    u32 spsr = *m_state.m_spsr_ptr;
                    m_state.switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                    m_state.m_cpsr = spsr;
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
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
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
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
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
                        set_carry(operand1 >= operand2);
                        update_overflow_sub(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
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
                        set_carry(operand2 >= operand1);
                        update_overflow_sub(result, operand2, operand1);
                        update_sign(result);
                        update_zero(result);
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

                        set_carry(result_long & 0x100000000);
                        update_overflow_add(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0101:
                {
                    // Addition with Carry
                    int carry2 = (m_state.m_cpsr >> 29) & 1;
                    u32 result = operand1 + operand2 + carry2;

                    if (set_flags)
                    {
                        u64 result_long = (u64)operand1 + (u64)operand2 + (u64)carry2;

                        set_carry(result_long & 0x100000000);
                        update_overflow_add(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0110:
                {
                    // Subtraction with Carry
                    int carry2 = (m_state.m_cpsr >> 29) & 1;
                    u32 result = operand1 - operand2 + carry2 - 1;

                    if (set_flags)
                    {
                        set_carry(operand1 >= (operand2 + carry2 - 1));
                        update_overflow_sub(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b0111:
                {
                    // Reverse Substraction with Carry
                    int carry2 = (m_state.m_cpsr >> 29) & 1;
                    u32 result = operand2 - operand1 + carry2 - 1;

                    if (set_flags)
                    {
                        set_carry(operand2 >= (operand1 + carry2 - 1));
                        update_overflow_sub(result, operand2, operand1);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b1000:
                {
                    // Bitwise AND flags only (TST)
                    u32 result = operand1 & operand2;

                    update_sign(result);
                    update_zero(result);
                    set_carry(carry);
                    break;
                }
                case 0b1001:
                {
                    // Bitwise EXOR flags only (TEQ)
                    u32 result = operand1 ^ operand2;

                    update_sign(result);
                    update_zero(result);
                    set_carry(carry);
                    break;
                }
                case 0b1010:
                {
                    // Subtraction flags only (CMP)
                    u32 result = operand1 - operand2;

                    set_carry(operand1 >= operand2);
                    update_overflow_sub(result, operand1, operand2);
                    update_sign(result);
                    update_zero(result);
                    break;
                }
                case 0b1011:
                {
                    // Addition flags only (CMN)
                    u32 result = operand1 + operand2;
                    u64 result_long = (u64)operand1 + (u64)operand2;

                    set_carry(result_long & 0x100000000);
                    update_overflow_add(result, operand1, operand2);
                    update_sign(result);
                    update_zero(result);
                    break;
                }
                case 0b1100:
                {
                    // Bitwise OR (ORR)
                    u32 result = operand1 | operand2;

                    if (set_flags)
                    {
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
                    }

                    reg(reg_dest) = result;
                    break;
                }
                case 0b1101:
                {
                    // Move into register (MOV)
                    if (set_flags)
                    {
                        update_sign(operand2);
                        update_zero(operand2);
                        set_carry(carry);
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
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
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
                        update_sign(not_operand2);
                        update_zero(not_operand2);
                        set_carry(carry);
                    }

                    reg(reg_dest) = not_operand2;
                    break;
                }
                }

                // Clear pipeline if r15 updated
                if (reg_dest == 15)
                {
                    m_Pipe.m_Flush = true;

                    // Emulate pipeline flush timings
                    cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::SequentialAccess(m_state.m_reg[15] + 4, ACCESS_WORD);
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
                   "Single Data Transfer, writeback to r15, r15=0x%x", m_state.m_reg[15]);
            ASSERT(write_back && !pre_indexed, LOG_WARN,
                   "Single Data Transfer, writeback and post-indexed, r15=0x%x", m_state.m_reg[15]);
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
                ASSERT(reg_offset == 15, LOG_WARN, "Single Data Transfer, r15 used as offset, r15=0x%x", m_state.m_reg[15]);
                #endif

                offset = reg(reg_offset);

                // Apply shift
                switch (shift)
                {
                case 0b00:
                    logical_shift_left(offset, amount, carry);
                    break;
                case 0b01:
                    logical_shift_right(offset, amount, carry, true);
                    break;
                case 0b10:
                    arithmetic_shift_right(offset, amount, carry, true);
                    break;
                case 0b11:
                    carry = m_state.m_cpsr & MASK_CFLAG;
                    rotate_right(offset, amount, carry, true);
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
                    cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                                  Memory::NonSequentialAccess(address, ACCESS_BYTE);
                }
                else
                {
                    reg(reg_dest) = ReadWordRotated(address);

                    // Calculate instruction timing
                    cycles += 1 + Memory::SequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                                  Memory::NonSequentialAccess(address, ACCESS_WORD);
                }

                // Writing to r15 causes a pipeline-flush.
                if (reg_dest == 15)
                {
                    m_Pipe.m_Flush = true;

                    // Emulate pipeline refill timings
                    cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::SequentialAccess(m_state.m_reg[15] + 4, ACCESS_WORD);
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
                    cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::NonSequentialAccess(address, ACCESS_BYTE);
                }
                else
                {
                    WriteWord(address, value);

                    // Calculate instruction timing
                    cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                              Memory::NonSequentialAccess(address, ACCESS_WORD);
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
            LOG(LOG_ERrotate_right, "Undefined instruction (0x%x), r15=0x%x", instruction, m_state.m_reg[15]);
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
            u32 address = m_state.m_reg[base_register];
            u32 address_old = address;
            int first_register = 0;
            int register_count = 0;
            int old_mode = 0;

            #ifdef DEBUG
            // Base register must not be r15
            ASSERT(base_register == 15, LOG_ERrotate_right, "ARM.11, rB=15, r15=0x%x", m_state.m_reg[15]);
            #endif

            // Switch to User mode if neccessary
            if (force_user_mode && (!load_instr || !(register_list & (1 << 15))))
            {
                #ifdef DEBUG
                ASSERT(write_back, LOG_ERrotate_right, "ARM.11, writeback and modeswitch, r15=0x%x", m_state.m_reg[15]);
                #endif

                // Save current mode and enter user mode
                old_mode = m_state.m_cpsr & MASK_MODE;
                ////SaveRegisters();
                ////m_state.m_cpsr = (m_state.m_cpsr & ~MASK_MODE) | MODE_USR;
                ////LoadRegisters();
                m_state.switch_mode(MODE_USR);

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
            cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_HWORD);

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

                            m_state.m_reg[i] = ReadWord(address);

                            if (i == 15)
                            {
                                if (force_user_mode)
                                {
                                    u32 spsr = *m_state.m_spsr_ptr;

                                    m_state.switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                                    m_state.m_cpsr = spsr;
                                }
                                m_Pipe.m_Flush = true;
                            }
                        }
                        else
                        {
                            if (i == first_register && i == base_register)
                                WriteWord(address, address_old);
                            else
                                WriteWord(address, m_state.m_reg[i]);
                        }

                        if (!pre_indexed)
                            address += 4;

                        if (write_back)
                            m_state.m_reg[base_register] = address;
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

                            m_state.m_reg[i] = ReadWord(address);

                            if (i == 15)
                            {
                                if (force_user_mode)
                                {
                                    ////SaveRegisters();
                                    ////m_state.m_cpsr = *m_state.m_spsr_ptr;
                                    ////LoadRegisters();
                                    u32 spsr = *m_state.m_spsr_ptr;

                                    m_state.switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                                    m_state.m_cpsr = spsr;
                                }
                                m_Pipe.m_Flush = true;
                            }
                        }
                        else
                        {
                            if (i == first_register && i == base_register)
                                WriteWord(address, address_old);
                            else
                                WriteWord(address, m_state.m_reg[i]);
                        }

                        if (!pre_indexed)
                            address -= 4;

                        if (write_back)
                            m_state.m_reg[base_register] = address;
                    }
                }
            }

            // Switch back to original mode.
            if (switched_mode)
            {
                ////SaveRegisters();
                ////m_state.m_cpsr = (m_state.m_cpsr & ~MASK_MODE) | old_mode;
                ////LoadRegisters();
                m_state.switch_mode(static_cast<cpu_mode>(old_mode));
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
                reg(14) = m_state.m_reg[15] - 4;
            }
            m_state.m_reg[15] += offset << 2;
            m_Pipe.m_Flush = true;
            return;
        }
        case ARM_16:
        {
            // ARM.16 Software interrupt
            u32 bios_call = ReadByte(m_state.m_reg[15] - 6);

            // Log to the console that we're issuing an interrupt.
            #ifdef DEBUG
            LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (arm)",
                bios_call, m_state.m_reg[0], m_state.m_reg[1], m_state.m_reg[2], m_state.m_reg[3], reg(14), m_state.m_reg[15]);
            #endif

            // Dispatch SWI, either HLE or BIOS.
            if (hle)
            {
                SWI(bios_call);
            }
            else
            {
                // Store return address in r14<svc>
                ////m_state.m_svc.m_r14 = m_state.m_reg[15] - 4;
                m_state.m_bank[BANK_SVC][BANK_R14] = m_state.m_reg[15] - 4;

                // Save program status and switch mode
                m_state.m_spsr[SPSR_SVC] = m_state.m_cpsr;
                ////SaveRegisters();
                ////m_state.m_cpsr = (m_state.m_cpsr & ~MASK_MODE) | MODE_SVC | MASK_IRQD;
                ////LoadRegisters();
                m_state.switch_mode(MODE_SVC);
                m_state.m_cpsr |= MASK_IRQD;

                // Jump to exception vector
                m_state.m_reg[15] = EXCPT_SWI;
                m_Pipe.m_Flush = true;
            }
            return;
        }
        }
    }
}
