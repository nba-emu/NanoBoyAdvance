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

namespace NanoboyAdvance
{
    ARM7::ARM7(Memory* memory, bool hle)
    {
        // Assign given memory instance to core
        this->memory = memory;

        // Map the static registers r0-r7, r15
        gprs[0] = &r0;
        gprs[1] = &r1;
        gprs[2] = &r2;
        gprs[3] = &r3;
        gprs[4] = &r4;
        gprs[5] = &r5;
        gprs[6] = &r6;
        gprs[7] = &r7;
        gprs[15] = &r15;
        RemapRegisters();

        // Skip bios boot logo
        r15 = 0x8000000;
        r13 = 0x3007F00;
        r13_svc = 0x3007FE0;
        r13_irq = 0x3007FA0;

        // Set hle flag
        this->hle = hle;
    }
    
    void ARM7::RemapRegisters()
    {
        switch (cpsr & 0x1F)
        {
        case User:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13;
            gprs[14] = &r14;
            pspsr = &spsr_def;
            break;
        case FIQ:
            gprs[8] = &r8_fiq;
            gprs[9] = &r9_fiq;
            gprs[10] = &r10_fiq;
            gprs[11] = &r11_fiq;
            gprs[12] = &r12_fiq;
            gprs[13] = &r13_fiq;
            gprs[14] = &r14_fiq;
            pspsr = &spsr_fiq;
            break;
        case IRQ:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13_irq;
            gprs[14] = &r14_irq;
            pspsr = &spsr_irq;
            break;
        case SVC:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13_svc;
            gprs[14] = &r14_svc;
            pspsr = &spsr_svc;
            break;
        case Abort:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13_abt;
            gprs[14] = &r14_abt;
            pspsr = &spsr_abt;
            break;
        case Undefined:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13_und;
            gprs[14] = &r14_und;
            pspsr = &spsr_und;
            break;
        case System:
            gprs[8] = &r8;
            gprs[9] = &r9;
            gprs[10] = &r10;
            gprs[11] = &r11;
            gprs[12] = &r12;
            gprs[13] = &r13;
            gprs[14] = &r14;
            pspsr = &spsr_def;
            break;
        }
    }

    void ARM7::LSL(uint& operand, uint amount, bool& carry)
    {
        // Nothing is done when the shift amount equals zero
        if (amount != 0)
        {
            // This way we easily bypass the 32 bits restriction on x86
            for (uint i = 0; i < amount; i++)
            {
                carry = operand & 0x80000000 ? true : false;
                operand <<= 1;
            }
        }
    }

    void ARM7::LSR(uint& operand, uint amount, bool& carry, bool immediate)
    {
        // LSR #0 equals to LSR #32
        amount = immediate & (amount == 0) ? 32 : amount;
        
        // Perform shift
        for (uint i = 0; i < amount; i++)
        {
            carry = operand & 1 ? true : false;
            operand >>= 1;
        }
    }

    void ARM7::ASR(uint& operand, uint amount, bool& carry, bool immediate)
    {
        uint sign_bit = operand & 0x80000000;

        // ASR #0 equals to ASR #32
        amount = immediate & (amount == 0) ? 32 : amount;

        // Perform shift
        for (uint i = 0; i < amount; i++)
        {
            carry = operand & 1 ? true : false;
            operand = (operand >> 1) | sign_bit;
        }
    }

    void ARM7::ROR(uint& operand, uint amount, bool& carry, bool immediate)
    {
        // ROR #0 equals to RRX #1
        if (amount != 0 || !immediate)
        {
            for (uint i = 1; i <= amount; i++)
            {
                uint high_bit = (operand & 1) ? 0x80000000 : 0;
                operand = (operand >> 1) | high_bit;
                carry = high_bit == 0x80000000;
            }
        }
        else
        {
            bool old_carry = carry;
            carry = (operand & 1) ? true : false;
            operand = (operand >> 1) | (old_carry ? 0x80000000 : 0);
        }
    }

    ubyte ARM7::ReadByte(uint offset)
    {
        TriggerMemoryBreakpoint(false, offset, 1);
        return memory->ReadByte(offset);
    }

    ushort ARM7::ReadHWord(uint offset)
    {
        // TODO: Proper handling (Mis-aligned LDRH,LDRSH)
        offset &= ~1;
        TriggerMemoryBreakpoint(false, offset, 2);
        return memory->ReadHWord(offset);
    }

    uint ARM7::ReadWord(uint offset)
    {
        offset &= ~3;
        TriggerMemoryBreakpoint(false, offset, 4);
        return memory->ReadWord(offset);
    }

    uint ARM7::ReadWordRotated(uint offset)
    {
        uint value = memory->ReadWord(offset & ~3);
        int amount = (offset & 3) * 8;
        TriggerMemoryBreakpoint(false, offset, 4);
        return amount == 0 ? value : ((value >> amount) | (value << (32 - amount)));
    }

    void ARM7::WriteByte(uint offset, ubyte value)
    {
        TriggerMemoryBreakpoint(true, offset, 1);
        memory->WriteByte(offset, value);
    }

    void ARM7::WriteHWord(uint offset, ushort value)
    {
        offset &= ~1;
        TriggerMemoryBreakpoint(true, offset, 2);
        memory->WriteHWord(offset, value);
    }

    void ARM7::WriteWord(uint offset, uint value)
    {
        offset &= ~3;
        TriggerMemoryBreakpoint(true, offset, 4);
        memory->WriteWord(offset, value);
    }
    
    void NanoboyAdvance::ARM7::TriggerMemoryBreakpoint(bool write, uint address, int size)
    {
        for (uint i = 0; i < breakpoints.size(); i++)
        {
            if ((breakpoints[i]->concerned_address >= address && breakpoints[i]->concerned_address <= address + size - 1) &&
                (
                 (breakpoints[i]->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::Access) ||
                 (breakpoints[i]->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::Read && !write) ||
                 (breakpoints[i]->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::Write && write)
                )
            )
            {
                hit_breakpoint = true;
                last_breakpoint = breakpoints[i];
                return;
            }
        }
    }

    void ARM7::TriggerSVCBreakpoint(uint bios_call)
    {
        for (uint i = 0; i < breakpoints.size(); i++)
        {
            ARM7Breakpoint* breakpoint = breakpoints[i];

            // Breakpoint bios call and type matching
            if (breakpoint->bios_call == bios_call && breakpoint->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::SVC)
            {
                hit_breakpoint = true;
                last_breakpoint = breakpoint;
            }
        }
    }

    void ARM7::Step()
    {
        bool thumb = (cpsr & Thumb) == Thumb;
        uint pc_page = r15 >> 24;

        // *Unset* breakpoint and crashed flag
        hit_breakpoint = false;
        crashed = false;

        // Debug when the program counter is in an unusual area        
        if (pc_page != 0 && pc_page != 2 && pc_page != 3 && pc_page != 6 && pc_page != 8)
        {
            crashed = true;
            crash_reason = CrashReason::BadPC;
        }

        // Handle code breakpoints
        for (uint i = 0; i < breakpoints.size(); i++)
        {
            if (breakpoints[i]->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::Execute &&
                breakpoints[i]->thumb_mode == thumb && 
                (r15 - (thumb ? 8 : 4)) == breakpoints[i]->concerned_address &&
                pipe_status >= 2)
            {
                hit_breakpoint = true;
                last_breakpoint = breakpoints[i];
                return;
            }
        }

        if (thumb)
        {
            r15 &= ~1;
            switch (pipe_status)
            {
            case 0:
                pipe_opcode[0] = memory->ReadHWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[0] : last_fetched_bios;
                break;
            case 1:
                pipe_opcode[1] = memory->ReadHWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[1] : last_fetched_bios;
                pipe_decode[0] = THUMBDecode(pipe_opcode[0]);
                break;
            case 2:
                pipe_opcode[2] = memory->ReadHWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[2] : last_fetched_bios;
                pipe_decode[1] = THUMBDecode(pipe_opcode[1]);
                THUMBExecute(pipe_opcode[0], pipe_decode[0]);
                break;
            case 3:
                pipe_opcode[0] = memory->ReadHWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[0] : last_fetched_bios;
                pipe_decode[2] = THUMBDecode(pipe_opcode[2]);
                THUMBExecute(pipe_opcode[1], pipe_decode[1]);
                break;
            case 4:
                pipe_opcode[1] = memory->ReadHWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[1] : last_fetched_bios;
                pipe_decode[0] = THUMBDecode(pipe_opcode[0]);
                THUMBExecute(pipe_opcode[2], pipe_decode[2]);
                break;
            }
        }
        else 
        {
            r15 &= ~3;
            switch (pipe_status)
            {
            case 0:
                pipe_opcode[0] = memory->ReadWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[0] : last_fetched_bios;
                break;
            case 1:
                pipe_opcode[1] = memory->ReadWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[1] : last_fetched_bios;
                pipe_decode[0] = ARMDecode(pipe_opcode[0]);
                break;
            case 2:
                pipe_opcode[2] = memory->ReadWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[2] : last_fetched_bios;
                pipe_decode[1] = ARMDecode(pipe_opcode[1]);
                ARMExecute(pipe_opcode[0], pipe_decode[0]);
                break;
            case 3:
                pipe_opcode[0] = memory->ReadWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[0] : last_fetched_bios;
                pipe_decode[2] = ARMDecode(pipe_opcode[2]);
                ARMExecute(pipe_opcode[1], pipe_decode[1]);
                break;
            case 4:
                pipe_opcode[1] = memory->ReadWord(r15);
                last_fetched_bios = r15 <= 0x3FFF ? pipe_opcode[1] : last_fetched_bios;
                pipe_decode[0] = ARMDecode(pipe_opcode[0]);
                ARMExecute(pipe_opcode[2], pipe_decode[2]);
                break;
            }
        }
        if (flush_pipe)
        {
            pipe_status = 0;
            flush_pipe = false;
            for (uint i = 0; i < breakpoints.size(); i++)
            {
                if (breakpoints[i]->breakpoint_type == ARM7Breakpoint::ARM7BreakpointType::StepOver)
                { 
                    hit_breakpoint = true;
                    last_breakpoint = breakpoints[i];
                }
            }
            return;
        }
        r15 += thumb ? 2 : 4;
        if (++pipe_status == 5)
        {
            pipe_status = 2;
        }
    }

    void ARM7::FireIRQ()
    {
        if ((cpsr & IRQDisable) == 0)
        {
            r14_irq = r15 - (cpsr & Thumb ? 2 : 4) + 4;
            spsr_irq = cpsr;
            cpsr = (cpsr & ~0x3F) | IRQ | IRQDisable;
            RemapRegisters();
            r15 = 0x18;
            flush_pipe = true;
        }
    }

    uint ARM7::GetGeneralRegister(int number)
    {
        return reg(number);
    }

    void ARM7::SetGeneralRegister(int number, uint value)
    {
        reg(number) = value;
    }

    uint ARM7::GetStatusRegister()
    {
        return cpsr;
    }

    uint ARM7::GetSavedStatusRegister()
    {
        return *pspsr;
    }

    void NanoboyAdvance::ARM7::SWI(int number)
    {
        switch (number)
        {
        // DIV
        case 0x06:
        {
            uint mod = r0 % r1;
            uint div = r0 / r1;
            r0 = div;
            r1 = mod;
            break;
        }
        // CpuSet
        case 0x0B:
        {
            uint source = r0;
            uint dest = r1;
            uint length = r2 & 0xFFFFF;
            bool fixed = r2 & (1 << 24) ? true : false;
            if (r2 & (1 << 26))
            {
                for (uint i = 0; i < length; i++)
                {
                    WriteWord(dest, ReadWord(source));
                    dest += 4;
                    if (!fixed) source += 4;
                }
            }
            else
            {
                for (uint i = 0; i < length; i++)
                {
                    WriteHWord(dest, ReadHWord(source));
                    dest += 2;
                    if (!fixed) source += 2;
                }
            }
            break;
        }
        // CpuFastSet
        case 0x0C:
        {
            uint source = r0;
            uint dest = r1;
            uint length = r2 & 0xFFFFF;
            bool fixed = r2 & (1 << 24) ? true : false;
            for (uint i = 0; i < length; i++)
            {
                WriteWord(dest, ReadWord(source));
                dest += 4;
                if (!fixed) source += 4;
            }
            break;
        }
        // LZ77UncompVRAM / LZ77UncompWRAM
        case 0x11:
        case 0x12:
        {
            int amount = memory->ReadWord(r0) >> 8;
            uint source = r0 + 4;
            uint dest = r1;
            while (amount > 0)
            {
                ubyte encoder = memory->ReadByte(source++);

                // Process 8 blocks encoded by the encoder
                for (int i = 7; i >= 0; i--)
                {
                    if (encoder & (1 << i))
                    {
                        // Compressed
                        ushort value = memory->ReadHWord(source);
                        uint disp = (value >> 8) | ((value & 0xF) << 8);
                        uint n = ((value >> 4) & 0xF) + 3;
                        source += 2;

                        for (uint j = 0; j < n; j++)
                        {
                            ushort value = memory->ReadByte(dest - disp - 1);
                            memory->WriteHWord(dest, value);
                            dest++;
                            amount--;
                            if (amount == 0)
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        // Uncompressed
                        ubyte value = memory->ReadByte(source++);
                        memory->WriteHWord(dest++, value);
                        amount--;
                        if (amount == 0)
                        {
                            return;
                        }
                    }
                }
            }
            break;
        }
        default:
            LOG(LOG_ERROR, "Unimplemented software interrupt 0x%x", number);
            break;
        }
    }
}
