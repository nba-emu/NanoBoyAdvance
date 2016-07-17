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

#include "arm.h"

namespace NanoboyAdvance
{
    ARM7::ARM7(GBAMemory* memory, bool hle)
    {
        // Assign given memory instance to core
        this->memory = memory;

        // Map the static registers r0-r7, r15
        gpr[0] = &r0;
        gpr[1] = &r1;
        gpr[2] = &r2;
        gpr[3] = &r3;
        gpr[4] = &r4;
        gpr[5] = &r5;
        gpr[6] = &r6;
        gpr[7] = &r7;
        gpr[15] = &r15;
        RemapRegisters();

        // Skip bios boot logo
        r15 = 0x8000000;
        r13 = 0x3007F00;
        r13_svc = 0x3007FE0;
        r13_irq = 0x3007FA0;

        // Set hle flag
        this->hle = hle;
    }
    
    u32 ARM7::GetGeneralRegister(Mode mode, int r)
    {
        Mode old_mode = (Mode)(cpsr & ModeField);
        u32 value;

        // Temporary switch to requested mode
        cpsr = (cpsr & ~ModeField) | (u32)mode;
        RemapRegisters();

        // Get value and switch back to correct mode
        value = reg(r);
        cpsr = (cpsr & ~ModeField) | (u32)old_mode;
        RemapRegisters();

        return value;
    }

    u32 ARM7::GetCurrentStatusRegister()
    {
        return cpsr;
    }

    u32 ARM7::GetSavedStatusRegister(Mode mode)
    {
        switch (mode)
        {
        case Mode::FIQ: return spsr_fiq;
        case Mode::SVC: return spsr_svc;
        case Mode::ABT: return spsr_abt;
        case Mode::IRQ: return spsr_irq;
        case Mode::UND: return spsr_und;        
        }
        return 0;
    }

    void ARM7::SetGeneralRegister(Mode mode, int r, u32 value)
    {
        Mode old_mode = (Mode)(cpsr & ModeField);

        // Temporary switch to requested mode
        cpsr = (cpsr & ~ModeField) | (u32)mode;
        RemapRegisters();

        // Write register and switch back
        reg(r) = value;
        cpsr = (cpsr & ~ModeField) | (u32)old_mode;
        RemapRegisters();
    }

    void ARM7::SetCurrentStatusRegister(u32 value)
    {
        cpsr = value;
    }
    
    void ARM7::SetSavedStatusRegister(Mode mode, u32 value)
    {
        switch (mode)
        {
        case Mode::FIQ: spsr_fiq = value; break;
        case Mode::SVC: spsr_svc = value; break;
        case Mode::ABT: spsr_abt = value; break;
        case Mode::IRQ: spsr_irq = value; break;
        case Mode::UND: spsr_und = value; break;        
        }
    }

    void ARM7::Step()
    {
        bool thumb = cpsr & ThumbFlag;

        // Forcibly align r15
        r15 &= thumb ? ~1 : ~3;

        // Dispatch instruction loading and execution
        switch (pipe.status)
        {
        case 0:    
            pipe.opcode[0] = thumb ? memory->ReadHWord(r15) : memory->ReadWord(r15);
            break;
        case 1:
            pipe.opcode[1] = thumb ? memory->ReadHWord(r15) : memory->ReadWord(r15);               
            break;
        case 2:
            if (thumb)
            {
                pipe.opcode[2] = memory->ReadHWord(r15);
                ExecuteThumb(pipe.opcode[0], DecodeThumb(pipe.opcode[0]));
                break;
            }
            pipe.opcode[2] = memory->ReadWord(r15);
            Execute(pipe.opcode[0], Decode(pipe.opcode[0]));
            break;
        case 3:
            if (thumb)
            {
                pipe.opcode[0] = memory->ReadHWord(r15);
                ExecuteThumb(pipe.opcode[1], DecodeThumb(pipe.opcode[1]));
                break;
            }
            pipe.opcode[0] = memory->ReadWord(r15);
            Execute(pipe.opcode[1], Decode(pipe.opcode[1]));
            break;
        case 4:
            if (thumb)
            {
                pipe.opcode[1] = memory->ReadHWord(r15);
                ExecuteThumb(pipe.opcode[2], DecodeThumb(pipe.opcode[2]));
                break;
            }
            pipe.opcode[1] = memory->ReadWord(r15);
            Execute(pipe.opcode[2], Decode(pipe.opcode[2]));
            break;
        }

        // Clear the pipeline if required
        if (pipe.flush)
        {
            pipe.status = 0;
            pipe.flush = false;
            return;
        }

        // Update pipeline status
        if (++pipe.status == 5) 
            pipe.status = 2;

        // Update instruction pointer
        r15 += thumb ? 2 : 4;
    }

    void ARM7::RemapRegisters()
    {
        switch (static_cast<Mode>(cpsr & ModeField))
        {
        case Mode::USR:
        case Mode::SYS:
            gpr[8] = &r8;
            gpr[9] = &r9;
            gpr[10] = &r10;
            gpr[11] = &r11;
            gpr[12] = &r12;
            gpr[13] = &r13;
            gpr[14] = &r14;
            pspsr = &spsr_def;
            break;
        case Mode::FIQ:
            gpr[8] = &r8_fiq;
            gpr[9] = &r9_fiq;
            gpr[10] = &r10_fiq;
            gpr[11] = &r11_fiq;
            gpr[12] = &r12_fiq;
            gpr[13] = &r13_fiq;
            gpr[14] = &r14_fiq;
            pspsr = &spsr_fiq;
            break;
        case Mode::IRQ:
            gpr[8] = &r8;
            gpr[9] = &r9;
            gpr[10] = &r10;
            gpr[11] = &r11;
            gpr[12] = &r12;
            gpr[13] = &r13_irq;
            gpr[14] = &r14_irq;
            pspsr = &spsr_irq;
            break;
        case Mode::SVC:
            gpr[8] = &r8;
            gpr[9] = &r9;
            gpr[10] = &r10;
            gpr[11] = &r11;
            gpr[12] = &r12;
            gpr[13] = &r13_svc;
            gpr[14] = &r14_svc;
            pspsr = &spsr_svc;
            break;
        case Mode::ABT:
            gpr[8] = &r8;
            gpr[9] = &r9;
            gpr[10] = &r10;
            gpr[11] = &r11;
            gpr[12] = &r12;
            gpr[13] = &r13_abt;
            gpr[14] = &r14_abt;
            pspsr = &spsr_abt;
            break;
        case Mode::UND:
            gpr[8] = &r8;
            gpr[9] = &r9;
            gpr[10] = &r10;
            gpr[11] = &r11;
            gpr[12] = &r12;
            gpr[13] = &r13_und;
            gpr[14] = &r14_und;
            pspsr = &spsr_und;
            break;
        }
    }

    void ARM7::RaiseIRQ()
    {
        if (((cpsr & IrqDisable) == 0) && pipe.status >= 2)
        {
            bool thumb = cpsr & ThumbFlag;

            #ifdef HARDCORE_DEBUG
            LOG(LOG_INFO, "Issued interrupt, r14_irq=0x%x, r15=0x%x", r14_irq, r15);
            #endif

            // "Useless" pipeline prefetch
            cycles += memory->SequentialAccess(r15, thumb ? GBAMemory::AccessSize::Hword : 
                                                            GBAMemory::AccessSize::Word);

            // Store return address in r14<irq>
            r14_irq = r15 - (thumb ? 4 : 8) + 4;

            // Save program status and switch mode
            spsr_irq = cpsr;
            cpsr = (cpsr & ~(ModeField | ThumbFlag)) | (u32)Mode::IRQ | IrqDisable;
            RemapRegisters();

            // Jump to exception vector
            r15 = (u32)Exception::Interrupt;
            pipe.status = 0;
            pipe.flush = false;

            // Emulate pipeline refill timings
            cycles += memory->NonSequentialAccess(r15, GBAMemory::AccessSize::Word) +
                      memory->SequentialAccess(r15 + 4, GBAMemory::AccessSize::Word);
        }
    }

    void NanoboyAdvance::ARM7::SWI(int number)
    {
        switch (number)
        {
        case 0x06:
        {
            if (r1 != 0)
            {
                u32 mod = r0 % r1;
                u32 div = r0 / r1;
                r0 = div;
                r1 = mod;
            } else 
            {
                LOG(LOG_ERROR, "Attempted division by zero.");
            }
            break;
        }
        case 0x05:
            r0 = 1;
            r1 = 1;
        case 0x04:
            memory->interrupt->ime = 1;

            // If r0 is one IF must be cleared
            if (r0 == 1)
                memory->interrupt->if_ = 0;

            // Sets GBA into halt state, waiting for specific interrupt(s) to occur.
            memory->intr_wait = true;
            memory->intr_wait_mask = r1;
            memory->halt_state = GBAMemory::HaltState::Halt;
            break;
        case 0x0B:
        {
            u32 source = r0;
            u32 dest = r1;
            u32 length = r2 & 0xFFFFF;
            bool fixed = r2 & (1 << 24) ? true : false;

            if (r2 & (1 << 26))
            {
                for (u32 i = 0; i < length; i++)
                {
                    WriteWord(dest, ReadWord(source));
                    dest += 4;
                    if (!fixed) source += 4;
                }
            }
            else
            {
                for (u32 i = 0; i < length; i++)
                {
                    WriteHWord(dest, ReadHWord(source));
                    dest += 2;
                    if (!fixed) source += 2;
                }
            }
            break;
        }
        case 0x0C:
        {
            u32 source = r0;
            u32 dest = r1;
            u32 length = r2 & 0xFFFFF;
            bool fixed = r2 & (1 << 24) ? true : false;

            for (u32 i = 0; i < length; i++)
            {
                WriteWord(dest, ReadWord(source));
                dest += 4;
                if (!fixed) source += 4;
            }
            break;
        }
        case 0x11:
        case 0x12:
        {
            int amount = memory->ReadWord(r0) >> 8;
            u32 source = r0 + 4;
            u32 dest = r1;

            while (amount > 0)
            {
                u8 encoder = memory->ReadByte(source++);

                // Process 8 blocks encoded by the encoder
                for (int i = 7; i >= 0; i--)
                {
                    if (encoder & (1 << i))
                    {
                        // Compressed
                        u16 value = memory->ReadHWord(source);
                        u32 disp = (value >> 8) | ((value & 0xF) << 8);
                        u32 n = ((value >> 4) & 0xF) + 3;
                        source += 2;

                        for (u32 j = 0; j < n; j++)
                        {
                            u16 value = memory->ReadByte(dest - disp - 1);
                            memory->WriteHWord(dest, value);
                            dest++;
                            amount--;
                            if (amount == 0)
                                return;
                        }
                    }
                    else
                    {
                        // Uncompressed
                        u8 value = memory->ReadByte(source++);
                        memory->WriteHWord(dest++, value);
                        amount--;
                        if (amount == 0)
                            return;
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

