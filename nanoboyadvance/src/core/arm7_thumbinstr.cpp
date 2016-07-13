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
    int ARM7::THUMBDecode(u16 instruction)
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
        else if ((instruction & 0xFF00) < 0xDF00)
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

    void ARM7::THUMBExecute(u16 instruction, int type)
    {
        // Actual execution
        switch (type)
        {
        case THUMB_1:
        {
            // THUMB.1 Move shifted register
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            u32 immediate_value = (instruction >> 6) & 0x1F;
            int opcode = (instruction >> 11) & 3;
            bool carry = cpsr & CarryFlag;

            reg(reg_dest) = reg(reg_source);

            switch (opcode)
            {
            case 0b00: // LSL
                LSL(reg(reg_dest), immediate_value, carry);
                AssertCarry(carry);
                break;
            case 0b01: // LSR
                LSR(reg(reg_dest), immediate_value, carry, true);
                AssertCarry(carry);
                break;
            case 0b10: // ASR
            {
                ASR(reg(reg_dest), immediate_value, carry, true);
                AssertCarry(carry);
                break;
            }
            }

            // Update sign and zero flag
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));

            // Update cycle counter
            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_2:
        {
            // THUMB.2 Add/subtract
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            u32 operand;

            // Either a register or an immediate
            if (instruction & (1 << 10))
                operand = (instruction >> 6) & 7;
            else
                operand = reg((instruction >> 6) & 7);

            // Determine wether to subtract or add
            if (instruction & (1 << 9))
            {
                u32 result = reg(reg_source) - operand;
                AssertCarry(reg(reg_source) >= operand);
                CalculateOverflowSub(result, reg(reg_source), operand);
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
            }
            else
            {
                u32 result = reg(reg_source) + operand;
                u64 result_long = (u64)(reg(reg_source)) + (u64)operand;
                AssertCarry(result_long & 0x100000000);
                CalculateOverflowAdd(result, reg(reg_source), operand);
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
            }

            // Update cycle counter
            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_3:
        {
            // THUMB.3 Move/compare/add/subtract immediate
            u32 immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;

            switch ((instruction >> 11) & 3)
            {
            case 0b00: // MOV
                CalculateSign(0);
                CalculateZero(immediate_value);
                reg(reg_dest) = immediate_value;
                break;
            case 0b01: // CMP
            {
                u32 result = reg(reg_dest) - immediate_value;
                AssertCarry(reg(reg_dest) >= immediate_value);
                CalculateOverflowSub(result, reg(reg_dest), immediate_value);
                CalculateSign(result);
                CalculateZero(result);
                break;
            }
            case 0b10: // ADD
            {
                u32 result = reg(reg_dest) + immediate_value;
                u64 result_long = (u64)(reg(reg_dest)) + (u64)immediate_value;
                AssertCarry(result_long & 0x100000000);
                CalculateOverflowAdd(result, reg(reg_dest), immediate_value);
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b11: // SUB
            {
                u32 result = reg(reg_dest) - immediate_value;
                AssertCarry(reg(reg_dest) >= immediate_value);
                CalculateOverflowSub(result, reg(reg_dest), immediate_value);
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
                break;
            }
            }

            // Update cycle counter
            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_4:
        {
            // THUMB.4 ALU operations
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;

            switch ((instruction >> 6) & 0xF)
            {
            case 0b0000: // AND
                reg(reg_dest) &= reg(reg_source);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                break;
            case 0b0001: // EOR
                reg(reg_dest) ^= reg(reg_source);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                break;
            case 0b0010: // LSL
            {
                u32 amount = reg(reg_source);
                bool carry = cpsr & CarryFlag;
                LSL(reg(reg_dest), amount, carry);
                AssertCarry(carry);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                cycles++;
                break;
            }
            case 0b0011: // LSR
            {
                u32 amount = reg(reg_source);
                bool carry = cpsr & CarryFlag;
                LSR(reg(reg_dest), amount, carry, false);
                AssertCarry(carry);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                cycles++;
                break;
            }
            case 0b0100: // ASR
            {
                u32 amount = reg(reg_source);
                bool carry = cpsr & CarryFlag;
                ASR(reg(reg_dest), amount, carry, false);
                AssertCarry(carry);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                cycles++;
                break;
            }
            case 0b0101: // ADC
            {
                int carry = (cpsr >> 29) & 1;
                u32 result = reg(reg_dest) + reg(reg_source) + carry;
                u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source)) + (u64)carry;
                AssertCarry(result_long & 0x100000000);
                CalculateOverflowAdd(result, reg(reg_dest), reg(reg_source) + carry);
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b0110: // SBC
            {
                int carry = (cpsr >> 29) & 1;
                u32 result = reg(reg_dest) - reg(reg_source) + carry - 1;
                AssertCarry(reg(reg_dest) >= reg(reg_source) + carry - 1);
                CalculateOverflowSub(result, reg(reg_dest), (reg(reg_source) + carry - 1));
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b0111: // ROR
            {
                u32 amount = reg(reg_source);
                bool carry = cpsr & CarryFlag;
                ROR(reg(reg_dest), amount, carry, false);
                AssertCarry(carry);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                cycles++;
                break;
            }
            case 0b1000: // TST
            {
                u32 result = reg(reg_dest) & reg(reg_source);
                CalculateSign(result);
                CalculateZero(result);
                break;
            }
            case 0b1001: // NEG
            {
                u32 result = 0 - reg(reg_source);
                AssertCarry(0 >= reg(reg_source));
                CalculateOverflowSub(result, 0, reg(reg_source));
                CalculateSign(result);
                CalculateZero(result);
                reg(reg_dest) = result;
                break;
            }
            case 0b1010: // CMP
            {
                u32 result = reg(reg_dest) - reg(reg_source);
                AssertCarry(reg(reg_dest) >= reg(reg_source));
                CalculateOverflowSub(result, reg(reg_dest), reg(reg_source));
                CalculateSign(result);
                CalculateZero(result);
                break;
            }
            case 0b1011: // CMN
            {
                u32 result = reg(reg_dest) + reg(reg_source);
                u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source));
                AssertCarry(result_long & 0x100000000);
                CalculateOverflowAdd(result, reg(reg_dest), reg(reg_source));
                CalculateSign(result);
                CalculateZero(result);
                break;
            }
            case 0b1100: // ORR
                reg(reg_dest) |= reg(reg_source);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                break;
            case 0b1101: // MUL
                // TODO: how to calc. the internal cycles?
                reg(reg_dest) *= reg(reg_source);
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                AssertCarry(false);
                break;
            case 0b1110: // BIC
                reg(reg_dest) &= ~(reg(reg_source));
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                break;
            case 0b1111: // MVN
                reg(reg_dest) = ~(reg(reg_source));
                CalculateSign(reg(reg_dest));
                CalculateZero(reg(reg_dest));
                break;
            }

            // Update cycle counter
            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_5:
        {
            // THUMB.5 Hi register operations/branch exchange
            int reg_dest = instruction & 7;
            int reg_source = (instruction >> 3) & 7;
            int opcode = (instruction >> 8) & 3;
            bool compare = false;
            u32 operand;

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

            operand = reg(reg_source);

            if (reg_source == 15)
                operand &= ~1;

            // Time next pipeline prefetch
            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);

            // Perform the actual operation
            switch (opcode)
            {
            case 0b00: // ADD
                reg(reg_dest) += operand;
                break;
            case 0b01: // CMP
            {
                u32 result = reg(reg_dest) - operand;
                AssertCarry(reg(reg_dest) >= operand);
                CalculateOverflowSub(result, reg(reg_dest), operand);
                CalculateSign(result);
                CalculateZero(result);
                compare = true;
                break;
            }
            case 0b10: // MOV
                reg(reg_dest) = operand;
                break;
            case 0b11: // BX
                // Switch instruction set?
                if (operand & 1)
                {
                    // Update r15
                    r15 = operand & ~1;

                    // Emulate pipeline refill cycles
                    cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                              memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);
                }
                else
                {
                    // Switch to ARM and update r15
                    cpsr &= ~Thumb;
                    r15 = operand & ~3;

                    // Emulate pipeline refill cycles
                    cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Word) +
                              memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Word);
                }

                // Flush pipeline
                flush_pipe = true;
                break;
            }

            if (reg_dest == 15 && !compare && opcode != 0b11)
            {
                // Flush pipeline
                reg(reg_dest) &= ~1;
                flush_pipe = true;

                // Emulate pipeline refill cycles
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);
            }
            break;
        }
        case THUMB_6:
        {
            // THUMB.6 PC-relative load
            u32 immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            u32 address = (r15 & ~2) + (immediate_value << 2);

            reg(reg_dest) = ReadWord(address);

            cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
            break;
        }
        case THUMB_7:
        {
            // THUMB.7 Load/store with register offset
            // TODO: check LDR(B) timings.
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            int reg_offset = (instruction >> 6) & 7;
            u32 address = reg(reg_base) + reg(reg_offset);

            switch ((instruction >> 10) & 3)
            {
            case 0b00: // STR
                WriteWord(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                break;
            case 0b01: // STRB
                WriteByte(address, reg(reg_dest) & 0xFF);
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Byte);
                break;
            case 0b10: // LDR
                reg(reg_dest) = ReadWordRotated(address);
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                break;
            case 0b11: // LDRB
                reg(reg_dest) = ReadByte(address);
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Byte);
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
            u32 address = reg(reg_base) + reg(reg_offset);

            switch ((instruction >> 10) & 3)
            {
            case 0b00: // STRH
                WriteHWord(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Hword);
                break;
            case 0b01: // LDSB
                reg(reg_dest) = ReadByte(address);

                if (reg(reg_dest) & 0x80)
                    reg(reg_dest) |= 0xFFFFFF00;
                
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Byte); 
                break;
            case 0b10: // LDRH
                reg(reg_dest) = ReadHWord(address);
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Hword);
                break;
            case 0b11: // LDSH
                reg(reg_dest) = ReadHWordSigned(address);

                // Uff... we should check wether ReadHWordSigned reads a
                // byte or a hword. However this should never really make difference.
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Hword);
                break;
            }

            break;
        }
        case THUMB_9:
        {
            // THUMB.9 Load store with immediate offset
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            u32 immediate_value = (instruction >> 6) & 0x1F;

            switch ((instruction >> 11) & 3)
            {
            case 0b00: { // STR
                u32 address = reg(reg_base) + (immediate_value << 2);
                WriteWord(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                break;
            }
            case 0b01: { // LDR
                u32 address = reg(reg_base) + (immediate_value << 2);
                reg(reg_dest) = ReadWordRotated(address);
                cycles += 1 + memory->SequentialAccess(r15,  GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                break;
            }
            case 0b10: { // STRB
                u32 address = reg(reg_base) + immediate_value;
                WriteByte(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Byte);
                break;
            }
            case 0b11: { // LDRB
                u32 address = reg(reg_base) + immediate_value;
                reg(reg_dest) = ReadByte(address);
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) + 
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Byte);
                break;
            }
            }
            break;
        }
        case THUMB_10:
        {
            // THUMB.10 Load/store halfword
            int reg_dest = instruction & 7;
            int reg_base = (instruction >> 3) & 7;
            u32 immediate_value = (instruction >> 6) & 0x1F;
            u32 address = reg(reg_base) + (immediate_value << 1);

            // Is the load bit set? (ldr)
            if (instruction & (1 << 11))
            {
                reg(reg_dest) = ReadHWord(address); // todo: alignment?
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
            }
            else
            {
                WriteHWord(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
            }
            break;
        }
        case THUMB_11:
        {
            // THUMB.11 SP-relative load/store
            u32 immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;
            u32 address = reg(13) + (immediate_value << 2);

            // Is the load bit set? (ldr)
            if (instruction & (1 << 11))
            {
                reg(reg_dest) = ReadWordRotated(address);
                cycles += 1 + memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                              memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
            }
            else
            {
                WriteWord(address, reg(reg_dest));
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
            }
            break;
        }
        case THUMB_12:
        {
            // THUMB.12 Load address
            u32 immediate_value = instruction & 0xFF;
            int reg_dest = (instruction >> 8) & 7;

            // Use stack pointer as base?
            if (instruction & (1 << 11)) 
                reg(reg_dest) = reg(13) + (immediate_value << 2); // sp
            else
                reg(reg_dest) = (r15 & ~2) + (immediate_value << 2); // pc

            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_13:
        {
            // THUMB.13 Add offset to stack pointer
            u32 immediate_value = (instruction & 0x7F) << 2;
            
            // Immediate-value is negative?
            if (instruction & 0x80)
                reg(13) -= immediate_value;
            else
                reg(13) += immediate_value;

            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_14:
        {
            // THUMB.14 push/pop registers
            // TODO: how to handle an empty register list?
            bool first_access = true;

            // One non-sequential prefetch cycle
            cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword);

            // Is this a POP instruction?
            if (instruction & (1 << 11))
            {
                // Iterate through the entire register list
                for (int i = 0; i <= 7; i++)
                {
                    // Pop into this register?
                    if (instruction & (1 << i))
                    {
                        u32 address = reg(13);

                        // Read word and update SP.
                        reg(i) = ReadWord(address);
                        reg(13) += 4;
                        
                        // Time the access based on if it's a first access
                        if (first_access) 
                        {
                            cycles += memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                            first_access = false;
                        }
                        else
                        {
                            cycles += memory->SequentialAccess(address, GBAMemory::AccessSize::Word);
                        }
                    }
                }

                // Also pop r15/pc if neccessary
                if (instruction & (1 << 8))
                {
                    u32 address = reg(13);

                    // Read word and update SP.
                    r15 = ReadWord(reg(13)) & ~1;
                    reg(13) += 4;

                    // Time the access based on if it's a first access
                    if (first_access) 
                    {
                        cycles += memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                        first_access = false;
                    }
                    else
                    {
                        cycles += memory->SequentialAccess(address, GBAMemory::AccessSize::Word);
                    }

                    flush_pipe = true;
                }
            }
            else
            {
                // Push r14/lr if neccessary
                if (instruction & (1 << 8))
                {
                    u32 address;

                    // Write word and update SP.
                    reg(13) -= 4;
                    address = reg(13);
                    WriteWord(address, reg(14));

                    // Time the access based on if it's a first access
                    if (first_access) 
                    {
                        cycles += memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                        first_access = false;
                    }
                    else
                    {
                        cycles += memory->SequentialAccess(address, GBAMemory::AccessSize::Word);
                    }
                }

                // Iterate through the entire register list
                for (int i = 7; i >= 0; i--)
                {
                    // Push this register?
                    if (instruction & (1 << i))
                    {
                        u32 address;

                        // Write word and update SP.
                        reg(13) -= 4;
                        address = reg(13);
                        WriteWord(address, reg(i));

                        // Time the access based on if it's a first access
                        if (first_access) 
                        {
                            cycles += memory->NonSequentialAccess(address, GBAMemory::AccessSize::Word);
                            first_access = false;
                        }
                        else
                        {
                            cycles += memory->SequentialAccess(address, GBAMemory::AccessSize::Word);
                        }
                    }
                }
            }
            break;
        }
        case THUMB_15:
        {
            // THUMB.15 Multiple load/store
            // TODO: Handle empty register list
            int reg_base = (instruction >> 8) & 7;
            bool write_back = true;
            u32 address = reg(reg_base);

            // Is the load bit set? (ldmia or stmia)
            if (instruction & (1 << 11))
            {
                cycles += 1 + memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                              memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);

                // Iterate through the entire register list
                for (int i = 0; i <= 7; i++)
                {
                    // Load to this register?
                    if (instruction & (1 << i))
                    {
                        reg(i) = ReadWord(address);
                        cycles += memory->SequentialAccess(address, GBAMemory::AccessSize::Word);
                        address += 4;
                    }
                }

                // Write back address into the base register if specified
                // and the base register is not in the register list
                if (write_back && !(instruction & (1 << reg_base)))
                    reg(reg_base) = address;
            }
            else
            {
                int first_register = 0;
                bool first_access = true;

                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword);

                // Find the first register
                for (int i = 0; i < 8; i++)
                {
                    if (instruction & (1 << i))
                    {
                        first_register = i;
                        break;
                    }
                }

                // Iterate through the entire register list
                for (int i = 0; i <= 7; i++)
                {
                    // Store this register?
                    if (instruction & (1 << i))
                    {
                        // Write register to the base address. If the current register is the
                        // base register and also the first register instead the original base is written.
                        if (i == reg_base && i == first_register)
                            WriteWord(reg(reg_base), address);
                        else
                            WriteWord(reg(reg_base), reg(i));

                        // Time the access based on if it's a first access
                        if (first_access)
                        {
                            cycles += memory->NonSequentialAccess(reg(reg_base), GBAMemory::AccessSize::Word);
                            first_access = false;
                        }
                        else
                        {
                            cycles += memory->SequentialAccess(reg(reg_base), GBAMemory::AccessSize::Word);
                        }

                        // Update base address
                        reg(reg_base) += 4;
                    }
                }
            }
            break;
        }
        case THUMB_16:
        {
            // THUMB.16 Conditional branch
            u32 signed_immediate = instruction & 0xFF;
            bool execute = false;

            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);

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
                    signed_immediate |= 0xFFFFFF00;
                
                // Update r15/pc and flush pipe
                r15 += (signed_immediate << 1);
                flush_pipe = true;

                // Emulate pipeline refill timings
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);
            }
            break;
        }
        case THUMB_17:
            // THUMB.17 Software Interrupt
            // TODO: move exception vectors to enum.
            //if ((cpsr & IRQDisable) == 0)
            {
                u8 bios_call = ReadByte(r15 - 4);

                // Log SWI to the console
                #ifdef DEBUG
                LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (thumb)", 
                    bios_call, r0, r1, r2, r3, reg(14), r15);
                #endif

                // "Useless" prefetch from r15 and pipeline refill timing.
                cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->NonSequentialAccess(8, GBAMemory::AccessSize::Word) +
                          memory->SequentialAccess(12, GBAMemory::AccessSize::Word);

                // Dispatch interrupt.
                if (hle)
                {
                    SWI(bios_call);
                }
                else
                {
                    r14_svc = r15 - 2;
                    spsr_svc = cpsr;
                    r15 = 8;
                    cpsr = (cpsr & ~0x3F) | (u32)ARM7Mode::SVC | IRQDisable;
                    RemapRegisters();
                    flush_pipe = true;
                }
            }
            break;
        case THUMB_18:
        {
            // THUMB.18 Unconditional branch
            u32 immediate_value = (instruction & 0x3FF) << 1;

            cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);

            // Sign-extend r15/pc displacement
            if (instruction & 0x400)
                immediate_value |= 0xFFFFF800;

            // Update r15/pc and flush pipe
            r15 += immediate_value;
            flush_pipe = true;

            //Emulate pipeline refill timings
            cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                      memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);
            break;
        }
        case THUMB_19:
        {
            u32 immediate_value = instruction & 0x7FF;

            // Branch with link consists of two instructions.
            if (instruction & (1 << 11))
            {
                u32 temp_pc = r15 - 2;
                u32 value = reg(14) + (immediate_value << 1);

                cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);

                // Update r15/pc
                value &= 0x7FFFFF;
                r15 &= ~0x7FFFFF;
                r15 |= value & ~1;

                // Store return address and flush pipe.
                reg(14) = temp_pc | 1;
                flush_pipe = true;

                //Emulate pipeline refill timings
                cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Hword) +
                          memory->SequentialAccess(r15 + 2, GBAMemory::AccessSize::Hword);
            }
            else
            {
                reg(14) = r15 + (immediate_value << 12);
                cycles += memory->SequentialAccess(r15, GBAMemory::AccessSize::Hword);
            }
            break;
        }
        }
    }
}
