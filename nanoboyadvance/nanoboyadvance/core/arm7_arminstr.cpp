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

    // TODO: If there is a cause for Ebola, Aids and Cancer, this is it (in other words, rewrite)
    //       Implement coprocessor operation instructions,
    //       Put string arrays in class definition and make them static
    string ARM7::ARMDisasm(uint base, uint instruction)
    {
        int type = ARMDecode(instruction);
        bool immediate = (instruction & (1 << 25)) == (1 << 25);
        bool preindexed = (instruction & (1 << 24)) == (1 << 24);
        bool addtobase = (instruction & (1 << 23)) == (1 << 23);
        bool writeback = (instruction & (1 << 21)) == (1 << 21);
        bool load = (instruction & (1 << 20)) == (1 << 20);
        stringstream mmemonic;
        string conditions[] = {
            "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
            "hi", "ls", "ge", "lt", "gt", "le", "",   "nv"
        };
        string shifts[] = { "lsl", "lsr", "asr", "ror" };
        string condition = conditions[instruction >> 28];
        switch (type)
        {
        case ARM_ERR:
            mmemonic << "<err>";
            break;
        case ARM_1:
        {
            // ARM.1 Multiply (accumulate) 
            int rm = instruction & 0xF;
            int rs = (instruction >> 8) & 0xF;
            int rn = (instruction >> 12) & 0xF;
            int rd = (instruction >> 16) & 0xF;
            if (instruction & (1 << 21))
            {
                mmemonic << "mla" << condition << (instruction & (1 << 20) ? "s " : " ") << "r" << rd << ", r" << rm<< ", r" << rs << ", r" << rn;
            }
            else
            {
                mmemonic << "mul" << condition << (instruction & (1 << 20) ? "s " : " ") << "r" << rd << ", r" << rm<< ", r" << rs;
            }
            break;
        }
        case ARM_2:
        {
            // ARM.2 Multiply (accumulate) long 
            int rm = instruction & 0xF;
            int rs = (instruction >> 8) & 0xF;
            int rd_lsw = (instruction >> 12) & 0xF;
            int rd_msw = (instruction >> 16) & 0xF;
            mmemonic << (instruction & (1 << 22) ? "s" : "u") << (instruction & (1 << 21) ? "mlal" : "mull") << condition << (instruction & (1 << 20) ? "s " : " ") <<
                        "r" << rd_lsw << ", r" << rd_msw << ", r" << rm << ", r" << rs;
            break;
        }
        case ARM_3:
            // ARM.3 Branch and exchange 
            mmemonic << "bx" << condition << " r" << (instruction & 0xF);
            break;
        case ARM_4:
        {
            // ARM.4 Single data swap 
            int rm = instruction & 0xF;
            int rd = (instruction >> 12) & 0xF;
            int rn = (instruction >> 16) & 0xF;
            mmemonic << "swp" << condition << (instruction & (1 << 22) ? "b " : " ") << "r" << rd << ", r" << rm << ", [r" << rn << "]";
            break;
        }
        // TODO: ARM.5 to ARM.7 reduce code redundancy 
        case ARM_5:
        {
            // ARM.5 Halfword data transfer, register offset 
            int rm = instruction & 0xF;
            int rd = (instruction >> 12) & 0xF;
            int rn = (instruction >> 16) & 0xF;
            mmemonic << (load ? "ldr" : "str") << condition << "h r" << rd << ", [r" << rn;
            if (preindexed)
            {
                mmemonic << ", " << (addtobase ? "r" : "-r") << rm << "]" << (writeback ? "!" : "");
            }
            else
            {
                mmemonic << "]" << ", " << (addtobase ? "r" : "-r") << rm << (writeback ? "!" : "");
            }
            break;
        }
        case ARM_6:
        {
            // ARM.6 Halfword data transfer, immediate offset 
            int offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
            int rd = (instruction >> 12) & 0xF;
            int rn = (instruction >> 16) & 0xF;
            mmemonic << (load ? "ldr" : "str") << condition << "h r" << rd << ", [r" << rn;
            if (preindexed)
            {
                mmemonic << ", " << (addtobase ? "#0x" : "-#0x") << hex << offset << dec << "]" << (writeback ? "!" : "");
            }
            else
            {
                mmemonic << "]" << ", " << (addtobase ? "#0x" : "-#0x") << hex << offset << dec << (writeback ? "!" : "");
            }
            break;
        }
        case ARM_7:
        {
            // ARM.7 Signed data transfer 
            int rd = (instruction >> 12) & 0xF;
            int rn = (instruction >> 16) & 0xF;
            bool immediate2 = (instruction & (1 << 22)) == (1 << 22);
            stringstream displacement;
            mmemonic << (load ? "ldr" : "str") << condition << (instruction & (1 << 5) ? "sh r" : "sb r") << rd << ", [r" << rn;
            if (immediate2)
            {
                int offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
                displacement << (addtobase ? "#0x" : "-#0x") << hex << offset << dec;
            }
            else
            {
                int rm = instruction & 0xF;
                displacement << (addtobase ? "r" : "-r") << rm;
            }
            if (preindexed)
            {
                mmemonic << ", " << displacement.str() << "]";
            }
            else
            {
                mmemonic << "], " << displacement.str();
            }
            mmemonic << (writeback ? "!" : "");
            break;
        }
        case ARM_8:
        {
            // ARM.8 Data processing and PSR transfer 
            int opcode = (instruction >> 21) & 0xF;
            bool setflags = (instruction & (1 << 20)) == (1 << 20);
            if (!setflags && opcode >= 0b1000 && opcode <= 0b1011)
            {
                // PSR transfer 
                switch ((instruction >> 16) & 0x3F)
                {
                // MRS (transfer PSR contents to a register) 
                case 0b001111:
                {
                    int rd = (instruction >> 12) & 0xF;
                    mmemonic << "mrs" << condition << " r" << rd << (instruction & (1 << 22) ? ", spsr_fsxc" : ", cpsr"); // is the _fsxc really correct? (vba appends this)
                    break;
                }
                // MSR (transfer register contents to PSR) 
                case 0b101001:
                {
                    int rm = instruction & 0xF;
                    mmemonic << "msr" << condition << (instruction & (1 << 22) ? " spsr_fc, r" : " cpsr_fc, r") << rm; // is the _fc really correct? (vba appends this)
                    break;
                }
                // MSR (transfer register contents or immediate value to PSR flag bits only) 
                case 0b101000:
                {
                    mmemonic << "msr" << condition << (instruction & (1 << 22) ? " spsr_f, " : " cpsr_f, "); // is the _f really correct? (vba appends this)
                    if (immediate)
                    {
                        int imm = instruction & 0xFF;
                        int ror = ((instruction >> 8) & 0xF) * 2;
                        uint result = (imm >> ror) | (imm << (32 - ror));
                        mmemonic << "#0x" << hex << result << dec;
                    }
                    else
                    {
                        mmemonic << "r" << (instruction & 0xF);
                    }
                    break;
                }
                // Undefined operation 
                default:
                    mmemonic << "<brokenpsr>";
                    break;
                }
            }
            else
            {
                // Data processing 
                int rd = (instruction >> 12) & 0xF;
                int rn = (instruction >> 16) & 0xF;
                string opcodes[] = {
                    "and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
                    "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
                };
                mmemonic << opcodes[opcode] << condition << (setflags ? "s " : " ");
                if (opcode >= 0b1000 && opcode <= 0b1011)
                {
                    mmemonic << "r" << rn;
                }
                else if (opcode == 0b1101 || opcode == 0b1111)
                {
                    mmemonic << "r" << rd;
                }
                else
                {
                    mmemonic << "r" << rd << ", r" << rn;
                }
                // Handle operand2 
                if (immediate)
                {
                    int imm = instruction & 0xFF;
                    int ror = ((instruction >> 8) & 0xF) * 2;
                    uint result = (imm >> ror) | (imm << (32 - ror));
                    mmemonic << ", #0x" << hex << result << dec;
                }
                else
                {
                    int rm = instruction & 0xF;
                    mmemonic << ", r" << rm << ", " << shifts[(instruction >> 5) & 3];
                    if (instruction & (1 << 4))
                    {
                        mmemonic << " r" << ((instruction >> 8) & 0xF);
                    }
                    else
                    {
                        mmemonic << " #0x" << hex << ((instruction >> 7) & 0x1F) << dec;
                    }
                }
            }
            break;
        }
        case ARM_9:
        {
            // ARM.9 Load/store register/unsigned byte 
            stringstream offset;
            int rd = (instruction >> 12) & 0xF;
            int rn = (instruction >> 16) & 0xF;
            mmemonic << (load ? "ldr" : "str") << condition << (instruction & (1 << 22) ? "b r" : " r") << rd << ", [r" << rn;
            // is it an immediate instruction? 
            if (immediate)
            {
                int rm = instruction & 0xF;
                offset << (addtobase ? "r" : "-r") << rm << ", " << shifts[(instruction >> 5) & 3];
                // is operand2 of the shift immediate or register? 
                if (instruction & (1 << 4))
                {
                    int rs = (instruction >> 8) & 0xF;
                    offset << " r" << rs;
                }
                else
                {
                    offset << " #0x" << hex << ((instruction >> 7) & 0x1F) << dec;
                }
            }
            else
            {
                offset << (addtobase ? "#0x" : "-#0x") << hex << (instruction & 0xFFF) << dec;
            }
            // is the instruction pre-indexed? 
            if (preindexed)
            {
                mmemonic << ", " << offset.str() << "]" << (writeback ? "!" : "");
            }
            else
            {
                mmemonic << "], " << offset.str() << (writeback ? "!" : "");
            }
            break;
        }
        case ARM_10:
            // ARM.10 Undefined 
            mmemonic << "<undef>";
            break;
        case ARM_11:
        {
            // ARM.11 Block data transfer 
            int rn = (instruction >> 16) & 0xF;
            bool forceuser = (instruction & (1 << 22)) == (1 << 22);
            bool listing = false;
            bool requirecomma = false;
            if (load && preindexed && addtobase)
            {
                mmemonic << "ldm" << condition << (rn == 13 ? "ed" : "ib");
            }
            else if (load && !preindexed && addtobase)
            {
                mmemonic << "ldm" << condition << (rn == 13 ? "fd" : "ia");
            }
            else if (load && preindexed && !addtobase)
            {
                mmemonic << "ldm" << condition << (rn == 13 ? "ea" : "db");
            }
            else if (load && !preindexed && !addtobase)
            {
                mmemonic << "ldm" << condition << (rn == 13 ? "fa" : "da");
            }
            else if (!load && preindexed && addtobase)
            {
                mmemonic << "stm" << condition << (rn == 13 ? "fa" : "ib");
            }
            else if (!load && !preindexed && addtobase)
            {
                mmemonic << "stm" << condition << (rn == 13 ? "ea" : "ia");
            }
            else if (!load && preindexed && !addtobase)
            {
                mmemonic << "stm" << condition << (rn == 13 ? "fd" : "db");
            }
            else
            {
                mmemonic << "stm" << condition << (rn == 13 ? "ed" : "da");
            }
            mmemonic << " r" << rn << (writeback ? "!, {" : ", {");
            for (int i = 0; i < 16; i++)
            {
                if (instruction & (1 << i))
                {
                    bool neighborset = false;
                    if (i != 15 && (instruction & (1 << (i + 1))))
                    {
                        neighborset = true;
                    }
                    if (neighborset && !listing)
                    {
                        listing = true;
                        mmemonic << (requirecomma ? ",r" : "r") << i << "-";
                    }
                    else if (!neighborset && listing)
                    {
                        listing = false;
                        mmemonic << "r" << i;
                    }
                    else if (!neighborset && !listing)
                    {
                        mmemonic << (requirecomma ? ",r" : "r") << i;
                    }
                    requirecomma = true;
                }
            }
            mmemonic << (forceuser ? "}^" : "}");
            break;
        }
        case ARM_12:
        {
            // ARM.12 Branch 
            int offset = instruction & 0xFFFFFF;
            if (offset & 0x800000)
            {
                offset |= 0xFF000000;
            }
            offset <<= 2;
            mmemonic << (instruction & (1 << 24) ? "bl" : "b") << condition << " 0x" << hex << (base + 8 + offset) << dec;
            break;
        }
        case ARM_13:
            // ARM.13 Coprocessor data transfer 
            mmemonic << "<coproc>";
            break;
        case ARM_14:
            // ARM.14 Coprocessor data operation 
            mmemonic << "<coproc2>";
            break;
        case ARM_15:
            // ARM.15 Coprocessor register transfer 
            mmemonic << "<coproc3>";
            break;
        case ARM_16:
            // ARM.16 Software interrupt 
            mmemonic << "swi" << condition << " #0x" << hex << (instruction & 0xFFFFFF) << dec;
            break;
        }
        return mmemonic.str();
    }
    
    void ARM7::ARMExecute(uint instruction, int type)
    {
        int condition = instruction >> 28;
        bool execute = false;
        LOG(LOG_INFO, "Executing %s, r15=0x%x", ARMDisasm(r15 - 8, instruction).c_str(), r15);
        switch (condition)
        {
        default:
            execute = true;
            break;
        }
        if (!execute)
        {
            return;
        }
        switch (type)
        {
        case ARM_1:
            // ARM.1 Multiply (accumulate)
            LOG(LOG_ERROR, "Unimplemented multiply (accumulate), r15=0x%x (0x%x)", r15, instruction);
            break;
        case ARM_2:
            // ARM.2 Multiply (accumulate) long
            LOG(LOG_ERROR, "Unimplemented multiply (accumulate) long, r15=0x%x (0x%x)", r15, instruction);
            break;
        case ARM_3:
        {
            // ARM.3 Branch and exchange
            int reg_address = instruction & 0xF;
            if (reg(reg_address) & 1)
            {
                LOG(LOG_ERROR, "Entered -unimplemented- thumb mode, r15=0x%x", r15);
                r15 = reg(reg_address) & ~(1);
                cpsr |= 0x20;
            }
            else
            {
                r15 = reg(reg_address) & ~(3);
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
            ASSERT(reg_source == 15 || reg_dest == 15 || reg_base == 15, LOG_WARN, "Single Data Transfer, thou shall not use r15, r15=0x%x", r15);
            if (swap_byte)
            {
                memory_value = memory->ReadByte(reg(reg_base)) & 0xFF;
                memory->WriteByte(reg(reg_base), reg(reg_source) & 0xFF);
                reg(reg_dest) = memory_value;
            }
            else
            {
                memory_value = memory->ReadWord(reg(reg_base));
                memory->WriteWord(reg(reg_base), reg(reg_source));
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
            ASSERT(reg_base == 15 && write_back, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not write to r15, r15=0x%x", r15);
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
                        value = memory->ReadHWord(address);
                        if (value & 0x8000)
                        {
                            value |= 0xFFFF0000;
                        }
                    }
                    else
                    {
                        value = memory->ReadByte(address);
                        if (value & 0x80)
                        {
                            value |= 0xFFFFFF00;
                        }
                    }
                    reg(reg_dest) = value;
                }
                else
                {
                    reg(reg_dest) = memory->ReadHWord(address);
                }
            }
            else
            {
                if (reg(reg_dest) == 15)
                {
                    memory->WriteHWord(address, r15 + 4);
                }
                else
                {
                    memory->WriteHWord(address, reg(reg_dest));
                }
            }

            // When the instruction either is pre-indexed and has the write-back bit or it's post-indexed we must writeback the calculated address 
            if (write_back || !pre_indexed)
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
        case ARM_9:
        case ARM_10:
        case ARM_11:
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
        case ARM_16:
            // ARM.16 Software interrupt
            r14_svc = r15 - 4;
            r15 = 0x8;
            flush_pipe = true;
            spsr_svc = cpsr;
            cpsr = (cpsr & ~0x1F) | SVC;
            RemapRegisters();
            break;
        }
    }
}