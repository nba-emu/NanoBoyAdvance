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
    ARM7::ARM7(Memory* memory)
    {
        this->memory = memory;
        // Zero-initialize stuff
        r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = r12 = r13 = r14 = r15 = 0;
        r8_fiq = r9_fiq = r10_fiq = r11_fiq = r12_fiq = r13_fiq = r14_fiq = 0;
        r13_svc = r14_svc = 0;
        r13_abt = r14_abt = 0;
        r13_irq = r14_irq = 0;
        r13_und = r14_und = 0;
        spsr_fiq = spsr_svc = spsr_abt = spsr_irq = spsr_und = spsr_def = 0;
        pipe_status = 0;
        flush_pipe = false;
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
        // Set the mode (system, thumb disabled)
        cpsr = System;
        RemapRegisters();
        // Testing
        r15 = 0x8000000;
        r13 = 0x3007F00;
        r13_svc = 0x3007FE0;
        r13_irq = 0x3007FA0;
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
            for (int i = 0; i < amount; i++)
            {
                carry = operand & 0x80000000 ? true : false;
                operand <<= 1;
            }
        }
    }

    void ARM7::LSR(uint& operand, uint amount, bool& carry)
    {
        // LSR #0 equals to LSR #32
        amount = amount == 0 ? 32 : amount;
        
        // Perform shift
        for (int i = 0; i < amount; i++)
        {
            carry = operand & 1 ? true : false;
            operand >>= 1;
        }
    }

    void ARM7::ASR(uint& operand, uint amount, bool& carry)
    {
        uint sign_bit = operand & 0x80000000;
        // ASR #0 equals to ASR #32
        amount = amount == 0 ? 32 : amount;

        // Perform shift
        if (amount != 0)
        {
            for (int i = 0; i < amount; i++)
            {
                carry = operand & 1 ? true : false;
                operand = (operand >> 1) | sign_bit;
            }
        }
        else
        {
            if (sign_bit)
            {
                operand = 0xFFFFFFFF;
                carry = true;
            }
            else
            {
                operand = 0;
                carry = false;
            }
        }
    }

    void ARM7::ROR(uint& operand, uint amount, bool& carry, bool thumb)
    {
        // In ARM mode RRX is performed when shift amount equals zero
        if (amount != 0 || thumb)
        {
            for (int i = 1; i <= amount; i++)
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
            operand = (operand >> 1) | old_carry ? 0x80000000 : 0;
        }
    }

    ubyte ARM7::ReadByte(uint offset)
    {
        // GBA specific case
        if (r15 > 0x4000 && offset < 0x4000)
        {
            return last_fetched_bios & 0xFF;
        }
        return memory->ReadByte(offset);
    }

    ushort ARM7::ReadHWord(uint offset)
    {
        // TODO: Proper handling (Mis-aligned LDRH,LDRSH)
        // GBA specific case
        if (r15 > 0x4000 && (offset & ~1) < 0x4000)
        {
            return last_fetched_bios & 0xFFFF;
        }
        return memory->ReadHWord(offset & ~1);
    }

    uint ARM7::ReadWord(uint offset)
    {
        // GBA specific case
        if (r15 > 0x4000 && (offset & ~3) < 0x4000)
        {
            return last_fetched_bios;
        }
        return memory->ReadWord(offset & ~3);
    }

    uint ARM7::ReadWordRotated(uint offset)
    {
        uint value = memory->ReadWord(offset & ~3);
        int amount = (offset & 3) * 8;

        // GBA specific case
        if (r15 > 0x4000 && (offset & ~3) < 0x4000)
        {
            return last_fetched_bios;
        }

        if (amount != 0)
        {
            return (value >> amount) | (value << (32 - amount));
        }
        else
        {
            return value;
        }
    }

    void ARM7::WriteByte(uint offset, ubyte value)
    {
        memory->WriteByte(offset, value);
    }

    void ARM7::WriteHWord(uint offset, ushort value)
    {
        memory->WriteHWord(offset & ~1, value);
    }

    void ARM7::WriteWord(uint offset, uint value)
    {
        memory->WriteWord(offset & ~3, value);
    }
    
    void ARM7::Step()
    {
        bool thumb = (cpsr & Thumb) == Thumb;
        uint pc_page = r15 >> 24;

        if (pc_page != 0 && pc_page != 2 && pc_page != 3 && pc_page != 6 && pc_page != 8)
        {
            LOG(LOG_ERROR, "Whoops! PC in suspicious area! This shouldn't happen!!! r15=0x%x", r15);
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
            r14_irq = r15 + 4; 
            spsr_irq = cpsr;
            cpsr = (cpsr & ~0x3F) | IRQ | IRQDisable;
            RemapRegisters();
            r15 = 0x18;
            flush_pipe = true;
        }
    }
}