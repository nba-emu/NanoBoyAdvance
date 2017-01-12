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

namespace GBA
{
    #include "thumb_lut.hpp"

    template <int imm, int type>
    void arm::thumb_1(u16 instruction)
    {
        // THUMB.1 Move shifted register
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        bool carry = m_cpsr & MASK_CFLAG;

        m_reg[reg_dest] = m_reg[reg_source];

        switch (type)
        {
        case 0: // LSL
            logical_shift_left(m_reg[reg_dest], imm, carry);
            set_carry(carry);
            break;
        case 1: // LSR
            logical_shift_right(m_reg[reg_dest], imm, carry, true);
            set_carry(carry);
            break;
        case 2: // ASR
        {
            arithmetic_shift_right(m_reg[reg_dest], imm, carry, true);
            set_carry(carry);
            break;
        }
        }

        // Update sign and zero flag
        update_sign(m_reg[reg_dest]);
        update_zero(m_reg[reg_dest]);
    }

    template <bool immediate, bool subtract, int field3>
    void arm::thumb_2(u16 instruction)
    {
        // THUMB.2 Add/subtract
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        u32 operand;

        // Either a register or an immediate
        if (immediate)
            operand = field3;
        else
            operand = m_reg[field3];

        // Determine wether to subtract or add
        if (subtract)
        {
            u32 result = m_reg[reg_source] - operand;

            // Calculate flags
            set_carry(m_reg[reg_source] >= operand);
            update_overflow_sub(result, m_reg[reg_source], operand);
            update_sign(result);
            update_zero(result);

            m_reg[reg_dest] = result;
        }
        else
        {
            u32 result = m_reg[reg_source] + operand;
            u64 result_long = (u64)(m_reg[reg_source]) + (u64)operand;

            // Calculate flags
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[reg_source], operand);
            update_sign(result);
            update_zero(result);

            m_reg[reg_dest] = result;
        }
    }

    template <int op, int reg_dest>
    void arm::thumb_3(u16 instruction)
    {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 immediate_value = instruction & 0xFF;

        switch (op)
        {
        case 0b00: // MOV
            update_sign(0);
            update_zero(immediate_value);
            m_reg[reg_dest] = immediate_value;
            break;
        case 0b01: // CMP
        {
            u32 result = m_reg[reg_dest] - immediate_value;
            set_carry(m_reg[reg_dest] >= immediate_value);
            update_overflow_sub(result, m_reg[reg_dest], immediate_value);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b10: // ADD
        {
            u32 result = m_reg[reg_dest] + immediate_value;
            u64 result_long = (u64)m_reg[reg_dest] + (u64)immediate_value;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[reg_dest], immediate_value);
            update_sign(result);
            update_zero(result);
            m_reg[reg_dest] = result;
            break;
        }
        case 0b11: // SUB
        {
            u32 result = m_reg[reg_dest] - immediate_value;
            set_carry(m_reg[reg_dest] >= immediate_value);
            update_overflow_sub(result, m_reg[reg_dest], immediate_value);
            update_sign(result);
            update_zero(result);
            m_reg[reg_dest] = result;
            break;
        }
        }
    }

    template <int op>
    void arm::thumb_4(u16 instruction)
    {
        // THUMB.4 ALU operations
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b0000: // AND
            m_reg[reg_dest] &= m_reg[reg_source];
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            break;
        case 0b0001: // EOR
            m_reg[reg_dest] ^= m_reg[reg_source];
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            break;
        case 0b0010: // LSL
        {
            u32 amount = m_reg[reg_source];
            bool carry = m_cpsr & MASK_CFLAG;
            logical_shift_left(m_reg[reg_dest], amount, carry);
            set_carry(carry);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            cycles++;
            break;
        }
        case 0b0011: // LSR
        {
            u32 amount = m_reg[reg_source];
            bool carry = m_cpsr & MASK_CFLAG;
            logical_shift_right(m_reg[reg_dest], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            cycles++;
            break;
        }
        case 0b0100: // ASR
        {
            u32 amount = m_reg[reg_source];
            bool carry = m_cpsr & MASK_CFLAG;
            arithmetic_shift_right(m_reg[reg_dest], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            cycles++;
            break;
        }
        case 0b0101: // ADC
        {
            int carry = (m_cpsr >> 29) & 1;
            u32 result = m_reg[reg_dest] + m_reg[reg_source] + carry;
            u64 result_long = (u64)(m_reg[reg_dest]) + (u64)(m_reg[reg_source]) + (u64)carry;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[reg_dest], m_reg[reg_source]);
            update_sign(result);
            update_zero(result);
            m_reg[reg_dest] = result;
            break;
        }
        case 0b0110: // SBC
        {
            int carry = (m_cpsr >> 29) & 1;
            u32 result = m_reg[reg_dest] - m_reg[reg_source] + carry - 1;
            set_carry(m_reg[reg_dest] >= (m_reg[reg_source] + carry - 1));
            update_overflow_sub(result, m_reg[reg_dest], m_reg[reg_source]);
            update_sign(result);
            update_zero(result);
            m_reg[reg_dest] = result;
            break;
        }
        case 0b0111: // ROR
        {
            u32 amount = m_reg[reg_source];
            bool carry = m_cpsr & MASK_CFLAG;
            rotate_right(m_reg[reg_dest], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            cycles++;
            break;
        }
        case 0b1000: // TST
        {
            u32 result = m_reg[reg_dest] & m_reg[reg_source];
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1001: // NEG
        {
            u32 result = 0 - m_reg[reg_source];
            set_carry(0 >= m_reg[reg_source]);
            update_overflow_sub(result, 0, m_reg[reg_source]);
            update_sign(result);
            update_zero(result);
            m_reg[reg_dest] = result;
            break;
        }
        case 0b1010: // CMP
        {
            u32 result = m_reg[reg_dest] - m_reg[reg_source];
            set_carry(m_reg[reg_dest] >= m_reg[reg_source]);
            update_overflow_sub(result, m_reg[reg_dest], m_reg[reg_source]);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1011: // CMN
        {
            u32 result = m_reg[reg_dest] + m_reg[reg_source];
            u64 result_long = (u64)(m_reg[reg_dest]) + (u64)(m_reg[reg_source]);
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[reg_dest], m_reg[reg_source]);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1100: // ORR
            m_reg[reg_dest] |= m_reg[reg_source];
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            m_reg[reg_dest] *= m_reg[reg_source];
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            set_carry(false);
            break;
        case 0b1110: // BIC
            m_reg[reg_dest] &= ~(m_reg[reg_source]);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            break;
        case 0b1111: // MVN
            m_reg[reg_dest] = ~(m_reg[reg_source]);
            update_sign(m_reg[reg_dest]);
            update_zero(m_reg[reg_dest]);
            break;
        }
    }

    template <int op, bool high1, bool high2>
    void arm::thumb_5(u16 instruction)
    {
        // THUMB.5 Hi register operations/branch exchange
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        bool compare = false;
        u32 operand;

        if (high1) reg_dest += 8;
        if (high2) reg_source += 8;

        operand = m_reg[reg_source];
        if (reg_source == 15) operand &= ~1;

        // Perform the actual operation
        switch (op)
        {
        case 0: // ADD
            m_reg[reg_dest] += operand;
            break;
        case 1: // CMP
        {
            u32 result = m_reg[reg_dest] - operand;
            set_carry(m_reg[reg_dest] >= operand);
            update_overflow_sub(result, m_reg[reg_dest], operand);
            update_sign(result);
            update_zero(result);
            compare = true;
            break;
        }
        case 2: // MOV
            m_reg[reg_dest] = operand;
            break;
        case 3: // BX
            // Bit0 being set in the address indicates
            // that the destination instruction is in THUMB mode.
            if (operand & 1)
            {
                m_reg[15] = operand & ~1;
            }
            else
            {
                m_cpsr &= ~MASK_THUMB;
                m_reg[15] = operand & ~3;
            }

            // Flush pipeline
            m_pipeline.m_needs_flush = true;
            break;
        }

        if (reg_dest == 15 && !compare && op != 0b11)
        {
            // Flush pipeline
            m_reg[reg_dest] &= ~1;
            m_pipeline.m_needs_flush = true;
        }
    }

    template <int reg_dest>
    void arm::thumb_6(u16 instruction)
    {
        // THUMB.6 PC-relative load
        u32 immediate_value = instruction & 0xFF;
        u32 address = (m_reg[15] & ~2) + (immediate_value << 2);

        m_reg[reg_dest] = read_word(address);
    }

    template <int op, int reg_offset>
    void arm::thumb_7(u16 instruction)
    {
        // THUMB.7 Load/store with register offset
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = m_reg[reg_base] + m_reg[reg_offset];

        switch (op)
        {
        case 0b00: // STR
            write_word(address, m_reg[reg_dest]);
            break;
        case 0b01: // STRB
            bus_write_byte(address, m_reg[reg_dest] & 0xFF);
            break;
        case 0b10: // LDR
            m_reg[reg_dest] = read_word_rotated(address);
            break;
        case 0b11: // LDRB
            m_reg[reg_dest] = bus_read_byte(address);
            break;
        }
    }

    template <int op, int reg_offset>
    void arm::thumb_8(u16 instruction)
    {
        // THUMB.8 Load/store sign-extended byte/halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = m_reg[reg_base] + m_reg[reg_offset];

        switch (op)
        {
        case 0b00: // STRH
            write_hword(address, m_reg[reg_dest]);
            break;
        case 0b01: // LDSB
            m_reg[reg_dest] = bus_read_byte(address);

            if (m_reg[reg_dest] & 0x80)
                m_reg[reg_dest] |= 0xFFFFFF00;
            break;
        case 0b10: // LDRH
            m_reg[reg_dest] = read_hword(address);
            break;
        case 0b11: // LDSH
            m_reg[reg_dest] = read_hword_signed(address);
            break;
        }
    }

    template <int op, int imm>
    void arm::thumb_9(u16 instruction)
    {
        // THUMB.9 Load store with immediate offset
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b00: { // STR
            u32 address = m_reg[reg_base] + (imm << 2);
            write_word(address, m_reg[reg_dest]);
            break;
        }
        case 0b01: { // LDR
            u32 address = m_reg[reg_base] + (imm << 2);
            m_reg[reg_dest] = read_word_rotated(address);
            break;
        }
        case 0b10: { // STRB
            u32 address = m_reg[reg_base] + imm;
            bus_write_byte(address, m_reg[reg_dest]);
            break;
        }
        case 0b11: { // LDRB
            u32 address = m_reg[reg_base] + imm;
            m_reg[reg_dest] = bus_read_byte(address);
            break;
        }
        }
    }

    template <bool load, int imm>
    void arm::thumb_10(u16 instruction)
    {
        // THUMB.10 Load/store halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 address = m_reg[reg_base] + (imm << 1);

        if (load)
        {
            m_reg[reg_dest] = read_hword(address); // TODO: alignment?
        }
        else
        {
            write_hword(address, m_reg[reg_dest]);
        }
    }

    template <bool load, int reg_dest>
    void arm::thumb_11(u16 instruction)
    {
        // THUMB.11 SP-relative load/store
        u32 immediate_value = instruction & 0xFF;
        u32 address = m_reg[13] + (immediate_value << 2);

        // Is the load bit set? (ldr)
        if (load)
        {
            m_reg[reg_dest] = read_word_rotated(address);
        }
        else
        {
            write_word(address, m_reg[reg_dest]);
        }
    }

    template <bool stackptr, int reg_dest>
    void arm::thumb_12(u16 instruction)
    {
        // THUMB.12 Load address
        u32 immediate_value = instruction & 0xFF;

        // Use stack pointer as base?
        if (stackptr)
            m_reg[reg_dest] = m_reg[13] + (immediate_value << 2); // sp
        else
            m_reg[reg_dest] = (m_reg[15] & ~2) + (immediate_value << 2); // pc
    }

    template <bool sub>
    void arm::thumb_13(u16 instruction)
    {
        // THUMB.13 Add offset to stack pointer
        u32 immediate_value = (instruction & 0x7F) << 2;

        // Immediate-value is negative?
        if (sub)
            m_reg[13] -= immediate_value;
        else
            m_reg[13] += immediate_value;
    }

    template <bool pop, bool rbit>
    void arm::thumb_14(u16 instruction)
    {
        // THUMB.14 push/pop registers
        // TODO: how to handle an empty register list?

        // Is this a POP instruction?
        if (pop)
        {
            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Pop into this register?
                if (instruction & (1 << i))
                {
                    u32 address = m_reg[13];

                    // Read word and update SP.
                    m_reg[i] = read_word(address);
                    m_reg[13] += 4;
                }
            }

            // Also pop r15/pc if neccessary
            if (rbit)
            {
                u32 address = m_reg[13];

                // Read word and update SP.
                m_reg[15] = read_word(m_reg[13]) & ~1;
                m_reg[13] += 4;

                m_pipeline.m_needs_flush = true;
            }
        }
        else
        {
            // Push r14/lr if neccessary
            if (rbit)
            {
                u32 address;

                // Write word and update SP.
                m_reg[13] -= 4;
                address = m_reg[13];
                write_word(address, m_reg[14]);
            }

            // Iterate through the entire register list
            for (int i = 7; i >= 0; i--)
            {
                // Push this register?
                if (instruction & (1 << i))
                {
                    u32 address;

                    // Write word and update SP.
                    m_reg[13] -= 4;
                    address = m_reg[13];
                    write_word(address, m_reg[i]);
                }
            }
        }
    }

    template <bool load, int reg_base>
    void arm::thumb_15(u16 instruction)
    {
        // THUMB.15 Multiple load/store
        // TODO: Handle empty register list
        bool write_back = true;
        u32 address = m_reg[reg_base];

        // Is the load bit set? (ldmia or stmia)
        if (load)
        {
            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Load to this register?
                if (instruction & (1 << i))
                {
                    m_reg[i] = read_word(address);
                    address += 4;
                }
            }

            // Write back address into the base register if specified
            // and the base register is not in the register list
            if (write_back && !(instruction & (1 << reg_base)))
                m_reg[reg_base] = address;
        }
        else
        {
            int first_register = 0;

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
                        write_word(m_reg[reg_base], address);
                    else
                        write_word(m_reg[reg_base], m_reg[i]);

                    // Update base address
                    m_reg[reg_base] += 4;
                }
            }
        }
    }

    template <int cond>
    void arm::thumb_16(u16 instruction)
    {
        // THUMB.16 Conditional branch
        u32 signed_immediate = instruction & 0xFF;

        // Check if the instruction will be executed
        switch (cond)
        {
        case 0x0: if (!(m_cpsr & MASK_ZFLAG))     return; break;
        case 0x1: if (  m_cpsr & MASK_ZFLAG)      return; break;
        case 0x2: if (!(m_cpsr & MASK_CFLAG))    return; break;
        case 0x3: if (  m_cpsr & MASK_CFLAG)     return; break;
        case 0x4: if (!(m_cpsr & MASK_NFLAG))     return; break;
        case 0x5: if (  m_cpsr & MASK_NFLAG)      return; break;
        case 0x6: if (!(m_cpsr & MASK_VFLAG)) return; break;
        case 0x7: if (  m_cpsr & MASK_VFLAG)  return; break;
        case 0x8: if (!(m_cpsr & MASK_CFLAG) ||  (m_cpsr & MASK_ZFLAG)) return; break;
        case 0x9: if ( (m_cpsr & MASK_CFLAG) && !(m_cpsr & MASK_ZFLAG)) return; break;
        case 0xA: if ((m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG)) return; break;
        case 0xB: if ((m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG)) return; break;
        case 0xC: if ((m_cpsr & MASK_ZFLAG) || ((m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG))) return; break;
        case 0xD: if (!(m_cpsr & MASK_ZFLAG) && ((m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG))) return; break;
        }

        // Sign-extend the immediate value if neccessary
        if (signed_immediate & 0x80)
            signed_immediate |= 0xFFFFFF00;

        // Update r15/pc and flush pipe
        m_reg[15] += (signed_immediate << 1);
        m_pipeline.m_needs_flush = true;
    }

    void arm::thumb_17(u16 instruction)
    {
        // THUMB.17 Software Interrupt
        u8 bios_call = bus_read_byte(m_reg[15] - 4);

        // Log SWI to the console
        #ifdef DEBUG
        LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (thumb)",
            bios_call, m_reg[0], m_reg[1], m_reg[2], m_reg[3], m_reg[14], m_reg[15]);
        #endif

        // Dispatch SWI, either HLE or BIOS.
        if (m_hle)
        {
            software_interrupt(bios_call);
        }
        else
        {
            // Store return address in r14<svc>
            ////m_svc.m_r14 = m_reg[15] - 2;
            m_bank[BANK_SVC][BANK_R14] = m_reg[15] - 2;

            // Save program status and switch mode
            ////SaveRegisters();
            m_spsr[SPSR_SVC] = m_cpsr;
            switch_mode(MODE_SVC);
            m_cpsr = (m_cpsr & ~MASK_THUMB) | MASK_IRQD;
            ////m_cpsr = (m_cpsr & ~(MASK_MODE | MASK_THUMB)) | MODE_SVC | MASK_IRQD;
            ////LoadRegisters();

            // Jump to exception vector
            m_reg[15] = EXCPT_SWI;
            m_pipeline.m_needs_flush = true;
        }
    }

    void arm::thumb_18(u16 instruction)
    {
        // THUMB.18 Unconditional branch
        u32 immediate_value = (instruction & 0x3FF) << 1;

        // Sign-extend r15/pc displacement
        if (instruction & 0x400)
            immediate_value |= 0xFFFFF800;

        // Update r15/pc and flush pipe
        m_reg[15] += immediate_value;
        m_pipeline.m_needs_flush = true;
    }

    template <bool h>
    void arm::thumb_19(u16 instruction)
    {
        // THUMB.19 Long branch with link.
        u32 immediate_value = instruction & 0x7FF;

        // Branch with link consists of two instructions.
        if (h)
        {
            u32 temp_pc = m_reg[15] - 2;
            u32 value = m_reg[14] + (immediate_value << 1);

            // Update r15/pc
            value &= 0x7FFFFF;
            m_reg[15] &= ~0x7FFFFF;
            m_reg[15] |= value & ~1;

            // Store return address and flush pipe.
            m_reg[14] = temp_pc | 1;
            m_pipeline.m_needs_flush = true;
        }
        else
        {
            m_reg[14] = m_reg[15] + (immediate_value << 12);
        }
    }
}
