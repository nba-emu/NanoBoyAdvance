/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include "arm.hpp"
#include "util/logger.hpp"

using namespace Util;

#define ADVANCE_PC ctx.r15 += 4;

#define REFILL_PIPELINE_A \
    ctx.pipe[0] = busRead32(ctx.r15,     M_NONSEQ);\
    ctx.pipe[1] = busRead32(ctx.r15 + 4, M_SEQ);\
    ctx.r15 += 8;

#define REFILL_PIPELINE_T \
    ctx.pipe[0] = busRead16(ctx.r15,     M_NONSEQ);\
    ctx.pipe[1] = busRead16(ctx.r15 + 2, M_SEQ);\
    ctx.r15 += 4;

namespace Core {

    enum class DataOp {
        AND = 0,
        EOR = 1,
        SUB = 2,
        RSB = 3,
        ADD = 4,
        ADC = 5,
        SBC = 6,
        RSC = 7,
        TST = 8,
        TEQ = 9,
        CMP = 10,
        CMN = 11,
        ORR = 12,
        MOV = 13,
        BIC = 14,
        MVN = 15
    };

    template <bool immediate, int opcode, bool _set_flags, int field4>
    void ARM::dataProcessingARM(u32 instruction) {
        bool set_flags = _set_flags; // make modifiable

        int reg_dst = (instruction >> 12) & 0xF;
        int reg_op1 = (instruction >> 16) & 0xF;

        u32 op1 = ctx.reg[reg_op1], op2 = 0;

        bool carry = ctx.cpsr & MASK_CFLAG;

        if (immediate) {
            int imm    =   instruction & 0xFF;
            int amount = ((instruction >> 8) & 0xF) << 1;

            if (amount != 0) {
                carry = (imm >> (amount - 1)) & 1;
                op2   = (imm >>  amount) | (imm << (32 - amount));
            } else {
                op2 = imm;
            }
        } else {
            u32  amount;
            int  reg_op2    = instruction    & 0xF;
            int  shift_type = ( field4 >> 1) & 3;
            bool shift_imm  = (~field4 >> 0) & 1;

            op2 = ctx.reg[reg_op2];

            if (!shift_imm) {
                amount = ctx.reg[(instruction >> 8) & 0xF];

                if (reg_op1 == 15) op1 += 4;
                if (reg_op2 == 15) op2 += 4;

                busInternalCycles(1);
            } else {
                amount = (instruction >> 7) & 0x1F;
            }

            applyShift(shift_type, op2, amount, carry, shift_imm);
        }

        if (reg_dst == 15) {
            if (set_flags) {
                u32 spsr = *ctx.p_spsr;
                switchMode(static_cast<Mode>(spsr & MASK_MODE));
                ctx.cpsr = spsr;
                set_flags = false;
            }
        }

        auto& out = ctx.reg[reg_dst];

        switch (static_cast<DataOp>(opcode)) {
        case DataOp::AND: out = opDataProc (op1 & op2, set_flags, set_flags, carry);          break;
        case DataOp::EOR: out = opDataProc (op1 ^ op2, set_flags, set_flags, carry);          break;
        case DataOp::SUB: out = opSUB      (op1, op2, set_flags);                             break;
        case DataOp::RSB: out = opSUB      (op2, op1, set_flags);                             break;
        case DataOp::ADD: out = opADD      (op1, op2, set_flags);                             break;
        case DataOp::ADC: out = opADC      (op1, op2, (  ctx.cpsr >>POS_CFLAG)&1, set_flags); break;
        case DataOp::SBC: out = opSBC      (op1, op2, (~(ctx.cpsr)>>POS_CFLAG)&1, set_flags); break;
        case DataOp::RSC: out = opSBC      (op2, op1, (~(ctx.cpsr)>>POS_CFLAG)&1, set_flags); break;
        case DataOp::TST:       opDataProc (op1 & op2, true, true, carry);                    break;
        case DataOp::TEQ:       opDataProc (op1 ^ op2, true, true, carry);                    break;
        case DataOp::CMP:       opSUB      (op1, op2, true);                                  break;
        case DataOp::CMN:       opADD      (op1, op2, true);                                  break;
        case DataOp::ORR: out = opDataProc (op1 | op2,  set_flags, set_flags, carry);         break;
        case DataOp::MOV: out = opDataProc (op2,        set_flags, set_flags, carry);         break;
        case DataOp::BIC: out = opDataProc (op1 & ~op2, set_flags, set_flags, carry);         break;
        case DataOp::MVN: out = opDataProc (~op2,       set_flags, set_flags, carry);         break;
        }

        if (reg_dst == 15) {
            if (ctx.cpsr & MASK_THUMB) {
                REFILL_PIPELINE_T;
            } else {
                REFILL_PIPELINE_A;
            }
        } else {
            ADVANCE_PC;
        }
    }

    template <bool immediate, bool use_spsr, bool to_status>
    void ARM::statusTransferARM(u32 instruction) {
        if (to_status) {
            u32 op;
            u32 mask = 0;

            // create mask based on fsxc-bits.
            if (instruction & (1 << 16)) mask |= 0x000000FF;
            if (instruction & (1 << 17)) mask |= 0x0000FF00;
            if (instruction & (1 << 18)) mask |= 0x00FF0000;
            if (instruction & (1 << 19)) mask |= 0xFF000000;

            // decode source operand
            if (immediate) {
                int imm = instruction & 0xFF;
                int amount = ((instruction >> 8) & 0xF) << 1;
                op = (imm >> amount) | (imm << (32 - amount));
            } else {
                op = ctx.reg[instruction & 0xF];
            }

            u32 value = op & mask;

            // write to cpsr or spsr
            if (!use_spsr) {
                // only switch mode if it actually gets written to
                if (mask & 0xFF) {
                    switchMode(static_cast<Mode>(value & MASK_MODE));
                }
                ctx.cpsr = (ctx.cpsr & ~mask) | value;
            } else {
                *ctx.p_spsr = (*ctx.p_spsr & ~mask) | value;
            }
        } else {
            int dst = (instruction >> 12) & 0xF;

            ctx.reg[dst] = use_spsr ? *ctx.p_spsr : ctx.cpsr;
        }

        // TODO: keep in mind that MSR can change the thumb bit!
        ADVANCE_PC;
    }

    template <bool accumulate, bool set_flags>
    void ARM::multiplyARM(u32 instruction) {
        u32 result;

        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst = (instruction >> 16) & 0xF;

        result = ctx.reg[op1] * ctx.reg[op2];

        if (accumulate) {
            int op3 = (instruction >> 12) & 0xF;
            result += ctx.reg[op3];
        }
        if (set_flags) {
            updateSignFlag(result);
            updateZeroFlag(result);
        }

        ctx.reg[dst] = result;

        ADVANCE_PC;
    }

    template <bool sign_extend, bool accumulate, bool set_flags>
    void ARM::multiplyLongARM(u32 instruction) {
        s64 result;
        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst_lo = (instruction >> 12) & 0xF;
        int dst_hi = (instruction >> 16) & 0xF;

        if (sign_extend) {
            s64 op1_value = ctx.reg[op1];
            s64 op2_value = ctx.reg[op2];

            // sign-extend operands
            if (op1_value & 0x80000000) op1_value |= 0xFFFFFFFF00000000;
            if (op2_value & 0x80000000) op2_value |= 0xFFFFFFFF00000000;

            result = op1_value * op2_value;
        } else {
            u64 uresult = (u64)ctx.reg[op1] * (u64)ctx.reg[op2];
            result = uresult;
        }

        if (accumulate) {
            s64 value = ctx.reg[dst_hi];

            // workaround required by x86.
            value <<= 16;
            value <<= 16;
            value |= ctx.reg[dst_lo];

            result += value;
        }

        u32 result_hi = result >> 32;

        ctx.reg[dst_lo] = result & 0xFFFFFFFF;
        ctx.reg[dst_hi] = result_hi;

        if (set_flags) {
            updateSignFlag(result_hi);
            updateZeroFlag(result);
        }

        ADVANCE_PC;
    }

    template <bool swap_byte>
    void ARM::singleDataSwapARM(u32 instruction) {
        u32 tmp;
        int src  = (instruction >>  0) & 0xF;
        int dst  = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;

        if (swap_byte) {
            tmp = read8(ctx.reg[base], M_NONE);
            write8(ctx.reg[base], (u8)ctx.reg[src], M_NONE);
        } else {
            tmp = read32(ctx.reg[base], M_ROTATE);
            write32(ctx.reg[base], ctx.reg[src], M_NONE);
        }

        ctx.reg[dst] = tmp;

        ADVANCE_PC;
    }

    void ARM::branchExchangeARM(u32 instruction) {
        u32 addr = ctx.reg[instruction & 0xF];

        if (addr & 1) {
            ctx.r15 = addr & ~1;
            ctx.cpsr |= MASK_THUMB;
            REFILL_PIPELINE_T;
        } else {
            ctx.r15 = addr & ~3;
            REFILL_PIPELINE_A;
        }
    }

    template <bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode>
    void ARM::halfwordSignedTransferARM(u32 instruction) {
        u32 off;
        int dst  = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = ctx.reg[base];

        if (immediate) {
            off = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
        } else {
            off = ctx.reg[instruction & 0xF];
        }

        if (pre_indexed) {
            addr += base_increment ? off : -off;
        }

        switch (opcode) {
        case 1:
            // load/store halfword
            if (load) {
                ctx.reg[dst] = read16(addr, M_ROTATE);
            } else {
                u32 value = ctx.reg[dst];

                // r15 is $+12 instead of $+8 because of internal prefetch.
                if (dst == 15) {
                    value += 4;
                }

                write16(addr, value, M_NONE);
            }
            break;
        case 2: {
            // load signed byte
            ctx.reg[dst] = read8(addr, M_SIGNED);
            break;
        }
        case 3: {
            // load signed halfword
            ctx.reg[dst] = read16(addr, M_SIGNED);
            break;
        }
        }

        if ((write_back || !pre_indexed) && base != dst) {
            if (!pre_indexed) {
                addr += base_increment ? off : -off;
            }
            ctx.reg[base] = addr;
        }

        ADVANCE_PC;
    }

    template <bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load>
    void ARM::singleTransferARM(u32 instruction) {
        u32 off;
        Mode old_mode;
        int dst  = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = ctx.reg[base];

        // post-indexing implicitly performs a write back.
        // in that case W-bit indicates wether user-mode register access should be enforced.
        if (!pre_indexed && write_back) {
            old_mode = static_cast<Mode>(ctx.cpsr & MASK_MODE);
            switchMode(MODE_USR);
        }

        // get address offset.
        if (immediate) {
            off = instruction & 0xFFF;
        } else {
            bool carry = ctx.cpsr & MASK_CFLAG;
            int shift = (instruction >> 5) & 3;
            u32 amount = (instruction >> 7) & 0x1F;

            off = ctx.reg[instruction & 0xF];
            applyShift(shift, off, amount, carry, true);
        }

        if (pre_indexed) {
            addr += base_increment ? off : -off;
        }

        if (load) {
            if (byte) {
                ctx.reg[dst] = read8(addr, M_NONE);
            } else {
                ctx.reg[dst] = read32(addr, M_ROTATE);
            }
        } else {
            u32 value = ctx.reg[dst];

            // r15 is $+12 now due to internal prefetch cycle.
            if (dst == 15) {
                value += 4;
            }

            if (byte) {
                write8(addr, (u8)value, M_NONE);
            } else {
                write32(addr, value, M_NONE);
            }
        }

        // writeback operation may not overwrite the destination register.
        if (base != dst) {
            if (!pre_indexed) {
                ctx.reg[base] += base_increment ? off : -off;

                // if user-mode was enforced, return to previous mode.
                //// TODO: check if this is really wrong. I think it is.
                //if (write_back) {
                //    switchMode(old_mode);
                //}
            }
            else if (write_back) {
                ctx.reg[base] = addr;
            }
        }

        // TODO: double-check if this is how it should be.
        // restore previous mode (if changed)
        if (!pre_indexed && write_back) {
            switchMode(old_mode);
        }

        // writes to r15 require a pipeline flush.
        if (load && dst == 15) { // eh
            REFILL_PIPELINE_A;
        } else {
            ADVANCE_PC;
        }
    }

    void ARM::undefinedInstARM(u32 instruction) {
        // save return address and program status
        ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 4;
        ctx.spsr[SPSR_SVC] = ctx.cpsr;

        // switch to UND mode and disable interrupts
        switchMode(MODE_UND);
        ctx.cpsr |= MASK_IRQD;

        // jump to exception vector
        ctx.r15 = EXCPT_UNDEFINED;

        REFILL_PIPELINE_A;
    }

    template <bool _pre_indexed, bool base_increment, bool user_mode, bool _write_back, bool load>
    void ARM::blockTransferARM(u32 instruction) {
        int first_register;
        int register_count = 0;
        int register_list = instruction & 0xFFFF;
        bool transfer_r15 = register_list & (1 << 15);
        bool pre_indexed = _pre_indexed;
        bool write_back = _write_back;

        int base = (instruction >> 16) & 0xF;

        Mode old_mode;
        bool mode_switched = false;

        // TODO: emulate empty register list

        if (user_mode && (!load || !transfer_r15)) {
            old_mode = static_cast<Mode>(ctx.cpsr & MASK_MODE);
            switchMode(MODE_USR);
            mode_switched = true;
        }

        // find first register in list and also count them.
        for (int i = 15; i >= 0; i--) {
            if (~register_list & (1 << i)) {
                continue;
            }

            first_register = i;
            register_count++;
        }

        u32 addr = ctx.reg[base];
        u32 addr_old = addr;

        // instruction internally works with incrementing addresses.
        if (!base_increment) {
            addr -= register_count * 4;
            ctx.reg[base] = addr;
            write_back = false;
            pre_indexed = !pre_indexed;
        }

        // process register list starting with the lowest address and register.
        for (int i = first_register; i < 16; i++) {
            if (~register_list & (1 << i)) {
                continue;
            }

            if (pre_indexed) {
                addr += 4;
            }

            if (load) {
                if (i == base) {
                    write_back = false;
                }

                ctx.reg[i] = read32(addr, M_NONE);

                if (i == 15) {
                    if (user_mode) {
                        u32 spsr = *ctx.p_spsr;
                        switchMode(static_cast<Mode>(spsr & MASK_MODE));
                        ctx.cpsr = spsr;
                    }
                }
            } else {
                if (i == first_register && i == base) {
                    write32(addr, addr_old, M_NONE);
                } else {
                    write32(addr, ctx.reg[i], M_NONE);
                }
            }

            if (!pre_indexed) {
                addr += 4;
            }

            if (write_back) {
                ctx.reg[base] = addr;
            }
        }

        if (mode_switched) {
            switchMode(old_mode);
        }

        // must reload pipeline if r15 was overwritten
        if (load && (register_list & (1 << 15))) {
            if (ctx.cpsr & MASK_THUMB) {
                REFILL_PIPELINE_T;
            } else {
                REFILL_PIPELINE_A;
            }
        } else {
            ADVANCE_PC;
        }
    }

    template <bool link>
    void ARM::branchARM(u32 instruction) {
        u32 off = instruction & 0xFFFFFF;

        if (off & 0x800000) {
            off |= 0xFF000000;
        }
        if (link) {
            ctx.reg[14] = ctx.r15 - 4;
        }

        ctx.r15 += off << 2;

        REFILL_PIPELINE_A;
    }

    void ARM::swiARM(u32 instruction) {
        u32 call_number = read8(ctx.r15 - 6, M_NONE);

        if (!fake_swi) {
            Logger::log<LOG_DEBUG>("SWI[{0:x}]: r0={1:x}, r1={2:x}, r2={3:x}", call_number, ctx.r0, ctx.r1, ctx.r2);

            // save return address and program status
            ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 4;
            ctx.spsr[SPSR_SVC] = ctx.cpsr;

            // switch to SVC mode and disable interrupts
            switchMode(MODE_SVC);
            ctx.cpsr |= MASK_IRQD;

            // jump to exception vector
            ctx.r15 = EXCPT_SWI;
            REFILL_PIPELINE_A;
        } else {
            handleSWI(call_number);
            ADVANCE_PC;
        }
    }

    // - Instruction Handler LUT -
    // Maps an instruction opcode to a precompiled handler method.
    const ARM::ARMInstruction ARM::arm_lut[4096] = {
        // 0x00X
        /* 0 */ &ARM::dataProcessingARM<0,0,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,0,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,0,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,0,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,0,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,0,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,0,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,0,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,0,0,8>,
        /* 9 */ &ARM::multiplyARM<0,0>,
        /* A */ &ARM::dataProcessingARM<0,0,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,0,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,0,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,0,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,0,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,0,0,0,3>,

        // 0x01X
        /* 0 */ &ARM::dataProcessingARM<0,0,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,0,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,0,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,0,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,0,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,0,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,0,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,0,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,0,1,8>,
        /* 9 */ &ARM::multiplyARM<0,1>,
        /* A */ &ARM::dataProcessingARM<0,0,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,0,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,0,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,0,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,0,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,0,0,1,3>,

        // 0x02X
        /* 0 */ &ARM::dataProcessingARM<0,1,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,1,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,1,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,1,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,1,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,1,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,1,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,1,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,1,0,8>,
        /* 9 */ &ARM::multiplyARM<1,0>,
        /* A */ &ARM::dataProcessingARM<0,1,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,0,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,1,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,0,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,1,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,0,1,0,3>,

        // 0x03X
        /* 0 */ &ARM::dataProcessingARM<0,1,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,1,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,1,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,1,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,1,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,1,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,1,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,1,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,1,1,8>,
        /* 9 */ &ARM::multiplyARM<1,1>,
        /* A */ &ARM::dataProcessingARM<0,1,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,0,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,1,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,0,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,1,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,0,1,1,3>,

        // 0x04X
        /* 0 */ &ARM::dataProcessingARM<0,2,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,2,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,2,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,2,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,2,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,2,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,2,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,2,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,2,0,8>,
        /* 9 */ &ARM::multiplyARM<0,0>,
        /* A */ &ARM::dataProcessingARM<0,2,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,1,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,2,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,1,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,2,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,1,0,0,3>,

        // 0x05X
        /* 0 */ &ARM::dataProcessingARM<0,2,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,2,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,2,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,2,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,2,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,2,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,2,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,2,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,2,1,8>,
        /* 9 */ &ARM::multiplyARM<0,1>,
        /* A */ &ARM::dataProcessingARM<0,2,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,1,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,2,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,1,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,2,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,1,0,1,3>,

        // 0x06X
        /* 0 */ &ARM::dataProcessingARM<0,3,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,3,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,3,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,3,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,3,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,3,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,3,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,3,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,3,0,8>,
        /* 9 */ &ARM::multiplyARM<1,0>,
        /* A */ &ARM::dataProcessingARM<0,3,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,1,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,3,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,1,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,3,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,1,1,0,3>,

        // 0x07X
        /* 0 */ &ARM::dataProcessingARM<0,3,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,3,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,3,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,3,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,3,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,3,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,3,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,3,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,3,1,8>,
        /* 9 */ &ARM::multiplyARM<1,1>,
        /* A */ &ARM::dataProcessingARM<0,3,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,0,1,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,3,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,0,1,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,3,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,0,1,1,1,3>,

        // 0x08X
        /* 0 */ &ARM::dataProcessingARM<0,4,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,4,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,4,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,4,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,4,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,4,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,4,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,4,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,4,0,8>,
        /* 9 */ &ARM::multiplyLongARM<0,0,0>,
        /* A */ &ARM::dataProcessingARM<0,4,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,0,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,4,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,0,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,4,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,0,0,0,3>,

        // 0x09X
        /* 0 */ &ARM::dataProcessingARM<0,4,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,4,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,4,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,4,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,4,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,4,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,4,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,4,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,4,1,8>,
        /* 9 */ &ARM::multiplyLongARM<0,0,1>,
        /* A */ &ARM::dataProcessingARM<0,4,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,0,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,4,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,0,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,4,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,0,0,1,3>,

        // 0x0AX
        /* 0 */ &ARM::dataProcessingARM<0,5,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,5,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,5,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,5,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,5,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,5,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,5,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,5,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,5,0,8>,
        /* 9 */ &ARM::multiplyLongARM<0,1,0>,
        /* A */ &ARM::dataProcessingARM<0,5,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,0,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,5,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,0,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,5,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,0,1,0,3>,

        // 0x0BX
        /* 0 */ &ARM::dataProcessingARM<0,5,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,5,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,5,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,5,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,5,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,5,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,5,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,5,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,5,1,8>,
        /* 9 */ &ARM::multiplyLongARM<0,1,1>,
        /* A */ &ARM::dataProcessingARM<0,5,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,0,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,5,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,0,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,5,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,0,1,1,3>,

        // 0x0CX
        /* 0 */ &ARM::dataProcessingARM<0,6,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,6,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,6,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,6,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,6,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,6,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,6,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,6,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,6,0,8>,
        /* 9 */ &ARM::multiplyLongARM<1,0,0>,
        /* A */ &ARM::dataProcessingARM<0,6,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,1,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,6,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,1,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,6,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,1,0,0,3>,

        // 0x0DX
        /* 0 */ &ARM::dataProcessingARM<0,6,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,6,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,6,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,6,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,6,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,6,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,6,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,6,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,6,1,8>,
        /* 9 */ &ARM::multiplyLongARM<1,0,1>,
        /* A */ &ARM::dataProcessingARM<0,6,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,1,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,6,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,1,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,6,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,1,0,1,3>,

        // 0x0EX
        /* 0 */ &ARM::dataProcessingARM<0,7,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,7,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,7,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,7,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,7,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,7,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,7,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,7,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,7,0,8>,
        /* 9 */ &ARM::multiplyLongARM<1,1,0>,
        /* A */ &ARM::dataProcessingARM<0,7,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,1,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,7,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,1,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,7,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,1,1,0,3>,

        // 0x0FX
        /* 0 */ &ARM::dataProcessingARM<0,7,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,7,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,7,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,7,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,7,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,7,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,7,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,7,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,7,1,8>,
        /* 9 */ &ARM::multiplyLongARM<1,1,1>,
        /* A */ &ARM::dataProcessingARM<0,7,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<0,1,1,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,7,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<0,1,1,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,7,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<0,1,1,1,1,3>,

        // 0x10X
        /* 0 */ &ARM::statusTransferARM<0,0,0>,
        /* 1 */ &ARM::statusTransferARM<0,0,0>,
        /* 2 */ &ARM::statusTransferARM<0,0,0>,
        /* 3 */ &ARM::statusTransferARM<0,0,0>,
        /* 4 */ &ARM::statusTransferARM<0,0,0>,
        /* 5 */ &ARM::statusTransferARM<0,0,0>,
        /* 6 */ &ARM::statusTransferARM<0,0,0>,
        /* 7 */ &ARM::statusTransferARM<0,0,0>,
        /* 8 */ &ARM::statusTransferARM<0,0,0>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::statusTransferARM<0,0,0>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,0,0,0,1>,
        /* C */ &ARM::statusTransferARM<0,0,0>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,0,0,0,2>,
        /* E */ &ARM::statusTransferARM<0,0,0>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,0,0,0,3>,

        // 0x11X
        /* 0 */ &ARM::dataProcessingARM<0,8,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,8,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,8,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,8,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,8,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,8,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,8,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,8,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,8,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,8,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,0,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,8,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,0,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,8,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,0,0,1,3>,

        // 0x12X
        /* 0 */ &ARM::statusTransferARM<0,0,1>,
        /* 1 */ &ARM::branchExchangeARM,
        /* 2 */ &ARM::statusTransferARM<0,0,1>,
        /* 3 */ &ARM::statusTransferARM<0,0,1>,
        /* 4 */ &ARM::statusTransferARM<0,0,1>,
        /* 5 */ &ARM::statusTransferARM<0,0,1>,
        /* 6 */ &ARM::statusTransferARM<0,0,1>,
        /* 7 */ &ARM::statusTransferARM<0,0,1>,
        /* 8 */ &ARM::statusTransferARM<0,0,1>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::statusTransferARM<0,0,1>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,0,1,0,1>,
        /* C */ &ARM::statusTransferARM<0,0,1>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,0,1,0,2>,
        /* E */ &ARM::statusTransferARM<0,0,1>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,0,1,0,3>,

        // 0x13X
        /* 0 */ &ARM::dataProcessingARM<0,9,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,9,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,9,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,9,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,9,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,9,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,9,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,9,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,9,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,9,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,0,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,9,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,0,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,9,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,0,1,1,3>,

        // 0x14X
        /* 0 */ &ARM::statusTransferARM<0,1,0>,
        /* 1 */ &ARM::statusTransferARM<0,1,0>,
        /* 2 */ &ARM::statusTransferARM<0,1,0>,
        /* 3 */ &ARM::statusTransferARM<0,1,0>,
        /* 4 */ &ARM::statusTransferARM<0,1,0>,
        /* 5 */ &ARM::statusTransferARM<0,1,0>,
        /* 6 */ &ARM::statusTransferARM<0,1,0>,
        /* 7 */ &ARM::statusTransferARM<0,1,0>,
        /* 8 */ &ARM::statusTransferARM<0,1,0>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::statusTransferARM<0,1,0>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,1,0,0,1>,
        /* C */ &ARM::statusTransferARM<0,1,0>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,1,0,0,2>,
        /* E */ &ARM::statusTransferARM<0,1,0>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,1,0,0,3>,

        // 0x15X
        /* 0 */ &ARM::dataProcessingARM<0,10,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,10,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,10,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,10,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,10,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,10,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,10,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,10,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,10,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,10,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,1,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,10,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,1,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,10,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,1,0,1,3>,

        // 0x16X
        /* 0 */ &ARM::statusTransferARM<0,1,1>,
        /* 1 */ &ARM::statusTransferARM<0,1,1>,
        /* 2 */ &ARM::statusTransferARM<0,1,1>,
        /* 3 */ &ARM::statusTransferARM<0,1,1>,
        /* 4 */ &ARM::statusTransferARM<0,1,1>,
        /* 5 */ &ARM::statusTransferARM<0,1,1>,
        /* 6 */ &ARM::statusTransferARM<0,1,1>,
        /* 7 */ &ARM::statusTransferARM<0,1,1>,
        /* 8 */ &ARM::statusTransferARM<0,1,1>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::statusTransferARM<0,1,1>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,1,1,0,1>,
        /* C */ &ARM::statusTransferARM<0,1,1>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,1,1,0,2>,
        /* E */ &ARM::statusTransferARM<0,1,1>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,1,1,0,3>,

        // 0x17X
        /* 0 */ &ARM::dataProcessingARM<0,11,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,11,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,11,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,11,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,11,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,11,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,11,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,11,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,11,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,11,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,0,1,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,11,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,0,1,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,11,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,0,1,1,1,3>,

        // 0x18X
        /* 0 */ &ARM::dataProcessingARM<0,12,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,12,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,12,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,12,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,12,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,12,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,12,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,12,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,12,0,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,12,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,0,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,12,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,0,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,12,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,0,0,0,3>,

        // 0x19X
        /* 0 */ &ARM::dataProcessingARM<0,12,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,12,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,12,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,12,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,12,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,12,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,12,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,12,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,12,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,12,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,0,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,12,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,0,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,12,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,0,0,1,3>,

        // 0x1AX
        /* 0 */ &ARM::dataProcessingARM<0,13,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,13,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,13,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,13,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,13,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,13,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,13,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,13,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,13,0,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,13,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,0,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,13,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,0,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,13,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,0,1,0,3>,

        // 0x1BX
        /* 0 */ &ARM::dataProcessingARM<0,13,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,13,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,13,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,13,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,13,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,13,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,13,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,13,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,13,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<0>,
        /* A */ &ARM::dataProcessingARM<0,13,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,0,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,13,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,0,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,13,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,0,1,1,3>,

        // 0x1CX
        /* 0 */ &ARM::dataProcessingARM<0,14,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,14,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,14,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,14,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,14,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,14,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,14,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,14,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,14,0,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,14,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,1,0,0,1>,
        /* C */ &ARM::dataProcessingARM<0,14,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,1,0,0,2>,
        /* E */ &ARM::dataProcessingARM<0,14,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,1,0,0,3>,

        // 0x1DX
        /* 0 */ &ARM::dataProcessingARM<0,14,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,14,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,14,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,14,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,14,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,14,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,14,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,14,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,14,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,14,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,1,0,1,1>,
        /* C */ &ARM::dataProcessingARM<0,14,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,1,0,1,2>,
        /* E */ &ARM::dataProcessingARM<0,14,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,1,0,1,3>,

        // 0x1EX
        /* 0 */ &ARM::dataProcessingARM<0,15,0,0>,
        /* 1 */ &ARM::dataProcessingARM<0,15,0,1>,
        /* 2 */ &ARM::dataProcessingARM<0,15,0,2>,
        /* 3 */ &ARM::dataProcessingARM<0,15,0,3>,
        /* 4 */ &ARM::dataProcessingARM<0,15,0,4>,
        /* 5 */ &ARM::dataProcessingARM<0,15,0,5>,
        /* 6 */ &ARM::dataProcessingARM<0,15,0,6>,
        /* 7 */ &ARM::dataProcessingARM<0,15,0,7>,
        /* 8 */ &ARM::dataProcessingARM<0,15,0,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,15,0,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,1,1,0,1>,
        /* C */ &ARM::dataProcessingARM<0,15,0,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,1,1,0,2>,
        /* E */ &ARM::dataProcessingARM<0,15,0,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,1,1,0,3>,

        // 0x1FX
        /* 0 */ &ARM::dataProcessingARM<0,15,1,0>,
        /* 1 */ &ARM::dataProcessingARM<0,15,1,1>,
        /* 2 */ &ARM::dataProcessingARM<0,15,1,2>,
        /* 3 */ &ARM::dataProcessingARM<0,15,1,3>,
        /* 4 */ &ARM::dataProcessingARM<0,15,1,4>,
        /* 5 */ &ARM::dataProcessingARM<0,15,1,5>,
        /* 6 */ &ARM::dataProcessingARM<0,15,1,6>,
        /* 7 */ &ARM::dataProcessingARM<0,15,1,7>,
        /* 8 */ &ARM::dataProcessingARM<0,15,1,8>,
        /* 9 */ &ARM::singleDataSwapARM<1>,
        /* A */ &ARM::dataProcessingARM<0,15,1,10>,
        /* B */ &ARM::halfwordSignedTransferARM<1,1,1,1,1,1>,
        /* C */ &ARM::dataProcessingARM<0,15,1,12>,
        /* D */ &ARM::halfwordSignedTransferARM<1,1,1,1,1,2>,
        /* E */ &ARM::dataProcessingARM<0,15,1,14>,
        /* F */ &ARM::halfwordSignedTransferARM<1,1,1,1,1,3>,

        // 0x20X
        /* 0 */ &ARM::dataProcessingARM<1,0,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,0,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,0,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,0,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,0,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,0,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,0,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,0,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,0,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,0,0,9>,
        /* A */ &ARM::dataProcessingARM<1,0,0,10>,
        /* B */ &ARM::dataProcessingARM<1,0,0,11>,
        /* C */ &ARM::dataProcessingARM<1,0,0,12>,
        /* D */ &ARM::dataProcessingARM<1,0,0,13>,
        /* E */ &ARM::dataProcessingARM<1,0,0,14>,
        /* F */ &ARM::dataProcessingARM<1,0,0,15>,

        // 0x21X
        /* 0 */ &ARM::dataProcessingARM<1,0,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,0,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,0,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,0,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,0,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,0,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,0,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,0,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,0,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,0,1,9>,
        /* A */ &ARM::dataProcessingARM<1,0,1,10>,
        /* B */ &ARM::dataProcessingARM<1,0,1,11>,
        /* C */ &ARM::dataProcessingARM<1,0,1,12>,
        /* D */ &ARM::dataProcessingARM<1,0,1,13>,
        /* E */ &ARM::dataProcessingARM<1,0,1,14>,
        /* F */ &ARM::dataProcessingARM<1,0,1,15>,

        // 0x22X
        /* 0 */ &ARM::dataProcessingARM<1,1,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,1,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,1,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,1,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,1,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,1,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,1,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,1,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,1,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,1,0,9>,
        /* A */ &ARM::dataProcessingARM<1,1,0,10>,
        /* B */ &ARM::dataProcessingARM<1,1,0,11>,
        /* C */ &ARM::dataProcessingARM<1,1,0,12>,
        /* D */ &ARM::dataProcessingARM<1,1,0,13>,
        /* E */ &ARM::dataProcessingARM<1,1,0,14>,
        /* F */ &ARM::dataProcessingARM<1,1,0,15>,

        // 0x23X
        /* 0 */ &ARM::dataProcessingARM<1,1,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,1,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,1,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,1,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,1,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,1,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,1,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,1,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,1,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,1,1,9>,
        /* A */ &ARM::dataProcessingARM<1,1,1,10>,
        /* B */ &ARM::dataProcessingARM<1,1,1,11>,
        /* C */ &ARM::dataProcessingARM<1,1,1,12>,
        /* D */ &ARM::dataProcessingARM<1,1,1,13>,
        /* E */ &ARM::dataProcessingARM<1,1,1,14>,
        /* F */ &ARM::dataProcessingARM<1,1,1,15>,

        // 0x24X
        /* 0 */ &ARM::dataProcessingARM<1,2,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,2,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,2,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,2,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,2,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,2,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,2,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,2,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,2,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,2,0,9>,
        /* A */ &ARM::dataProcessingARM<1,2,0,10>,
        /* B */ &ARM::dataProcessingARM<1,2,0,11>,
        /* C */ &ARM::dataProcessingARM<1,2,0,12>,
        /* D */ &ARM::dataProcessingARM<1,2,0,13>,
        /* E */ &ARM::dataProcessingARM<1,2,0,14>,
        /* F */ &ARM::dataProcessingARM<1,2,0,15>,

        // 0x25X
        /* 0 */ &ARM::dataProcessingARM<1,2,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,2,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,2,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,2,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,2,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,2,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,2,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,2,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,2,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,2,1,9>,
        /* A */ &ARM::dataProcessingARM<1,2,1,10>,
        /* B */ &ARM::dataProcessingARM<1,2,1,11>,
        /* C */ &ARM::dataProcessingARM<1,2,1,12>,
        /* D */ &ARM::dataProcessingARM<1,2,1,13>,
        /* E */ &ARM::dataProcessingARM<1,2,1,14>,
        /* F */ &ARM::dataProcessingARM<1,2,1,15>,

        // 0x26X
        /* 0 */ &ARM::dataProcessingARM<1,3,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,3,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,3,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,3,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,3,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,3,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,3,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,3,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,3,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,3,0,9>,
        /* A */ &ARM::dataProcessingARM<1,3,0,10>,
        /* B */ &ARM::dataProcessingARM<1,3,0,11>,
        /* C */ &ARM::dataProcessingARM<1,3,0,12>,
        /* D */ &ARM::dataProcessingARM<1,3,0,13>,
        /* E */ &ARM::dataProcessingARM<1,3,0,14>,
        /* F */ &ARM::dataProcessingARM<1,3,0,15>,

        // 0x27X
        /* 0 */ &ARM::dataProcessingARM<1,3,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,3,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,3,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,3,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,3,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,3,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,3,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,3,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,3,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,3,1,9>,
        /* A */ &ARM::dataProcessingARM<1,3,1,10>,
        /* B */ &ARM::dataProcessingARM<1,3,1,11>,
        /* C */ &ARM::dataProcessingARM<1,3,1,12>,
        /* D */ &ARM::dataProcessingARM<1,3,1,13>,
        /* E */ &ARM::dataProcessingARM<1,3,1,14>,
        /* F */ &ARM::dataProcessingARM<1,3,1,15>,

        // 0x28X
        /* 0 */ &ARM::dataProcessingARM<1,4,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,4,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,4,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,4,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,4,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,4,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,4,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,4,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,4,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,4,0,9>,
        /* A */ &ARM::dataProcessingARM<1,4,0,10>,
        /* B */ &ARM::dataProcessingARM<1,4,0,11>,
        /* C */ &ARM::dataProcessingARM<1,4,0,12>,
        /* D */ &ARM::dataProcessingARM<1,4,0,13>,
        /* E */ &ARM::dataProcessingARM<1,4,0,14>,
        /* F */ &ARM::dataProcessingARM<1,4,0,15>,

        // 0x29X
        /* 0 */ &ARM::dataProcessingARM<1,4,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,4,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,4,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,4,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,4,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,4,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,4,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,4,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,4,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,4,1,9>,
        /* A */ &ARM::dataProcessingARM<1,4,1,10>,
        /* B */ &ARM::dataProcessingARM<1,4,1,11>,
        /* C */ &ARM::dataProcessingARM<1,4,1,12>,
        /* D */ &ARM::dataProcessingARM<1,4,1,13>,
        /* E */ &ARM::dataProcessingARM<1,4,1,14>,
        /* F */ &ARM::dataProcessingARM<1,4,1,15>,

        // 0x2AX
        /* 0 */ &ARM::dataProcessingARM<1,5,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,5,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,5,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,5,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,5,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,5,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,5,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,5,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,5,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,5,0,9>,
        /* A */ &ARM::dataProcessingARM<1,5,0,10>,
        /* B */ &ARM::dataProcessingARM<1,5,0,11>,
        /* C */ &ARM::dataProcessingARM<1,5,0,12>,
        /* D */ &ARM::dataProcessingARM<1,5,0,13>,
        /* E */ &ARM::dataProcessingARM<1,5,0,14>,
        /* F */ &ARM::dataProcessingARM<1,5,0,15>,

        // 0x2BX
        /* 0 */ &ARM::dataProcessingARM<1,5,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,5,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,5,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,5,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,5,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,5,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,5,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,5,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,5,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,5,1,9>,
        /* A */ &ARM::dataProcessingARM<1,5,1,10>,
        /* B */ &ARM::dataProcessingARM<1,5,1,11>,
        /* C */ &ARM::dataProcessingARM<1,5,1,12>,
        /* D */ &ARM::dataProcessingARM<1,5,1,13>,
        /* E */ &ARM::dataProcessingARM<1,5,1,14>,
        /* F */ &ARM::dataProcessingARM<1,5,1,15>,

        // 0x2CX
        /* 0 */ &ARM::dataProcessingARM<1,6,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,6,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,6,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,6,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,6,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,6,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,6,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,6,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,6,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,6,0,9>,
        /* A */ &ARM::dataProcessingARM<1,6,0,10>,
        /* B */ &ARM::dataProcessingARM<1,6,0,11>,
        /* C */ &ARM::dataProcessingARM<1,6,0,12>,
        /* D */ &ARM::dataProcessingARM<1,6,0,13>,
        /* E */ &ARM::dataProcessingARM<1,6,0,14>,
        /* F */ &ARM::dataProcessingARM<1,6,0,15>,

        // 0x2DX
        /* 0 */ &ARM::dataProcessingARM<1,6,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,6,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,6,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,6,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,6,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,6,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,6,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,6,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,6,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,6,1,9>,
        /* A */ &ARM::dataProcessingARM<1,6,1,10>,
        /* B */ &ARM::dataProcessingARM<1,6,1,11>,
        /* C */ &ARM::dataProcessingARM<1,6,1,12>,
        /* D */ &ARM::dataProcessingARM<1,6,1,13>,
        /* E */ &ARM::dataProcessingARM<1,6,1,14>,
        /* F */ &ARM::dataProcessingARM<1,6,1,15>,

        // 0x2EX
        /* 0 */ &ARM::dataProcessingARM<1,7,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,7,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,7,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,7,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,7,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,7,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,7,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,7,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,7,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,7,0,9>,
        /* A */ &ARM::dataProcessingARM<1,7,0,10>,
        /* B */ &ARM::dataProcessingARM<1,7,0,11>,
        /* C */ &ARM::dataProcessingARM<1,7,0,12>,
        /* D */ &ARM::dataProcessingARM<1,7,0,13>,
        /* E */ &ARM::dataProcessingARM<1,7,0,14>,
        /* F */ &ARM::dataProcessingARM<1,7,0,15>,

        // 0x2FX
        /* 0 */ &ARM::dataProcessingARM<1,7,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,7,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,7,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,7,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,7,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,7,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,7,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,7,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,7,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,7,1,9>,
        /* A */ &ARM::dataProcessingARM<1,7,1,10>,
        /* B */ &ARM::dataProcessingARM<1,7,1,11>,
        /* C */ &ARM::dataProcessingARM<1,7,1,12>,
        /* D */ &ARM::dataProcessingARM<1,7,1,13>,
        /* E */ &ARM::dataProcessingARM<1,7,1,14>,
        /* F */ &ARM::dataProcessingARM<1,7,1,15>,

        // 0x30X
        /* 0 */ &ARM::statusTransferARM<1,0,0>,
        /* 1 */ &ARM::statusTransferARM<1,0,0>,
        /* 2 */ &ARM::statusTransferARM<1,0,0>,
        /* 3 */ &ARM::statusTransferARM<1,0,0>,
        /* 4 */ &ARM::statusTransferARM<1,0,0>,
        /* 5 */ &ARM::statusTransferARM<1,0,0>,
        /* 6 */ &ARM::statusTransferARM<1,0,0>,
        /* 7 */ &ARM::statusTransferARM<1,0,0>,
        /* 8 */ &ARM::statusTransferARM<1,0,0>,
        /* 9 */ &ARM::statusTransferARM<1,0,0>,
        /* A */ &ARM::statusTransferARM<1,0,0>,
        /* B */ &ARM::statusTransferARM<1,0,0>,
        /* C */ &ARM::statusTransferARM<1,0,0>,
        /* D */ &ARM::statusTransferARM<1,0,0>,
        /* E */ &ARM::statusTransferARM<1,0,0>,
        /* F */ &ARM::statusTransferARM<1,0,0>,

        // 0x31X
        /* 0 */ &ARM::dataProcessingARM<1,8,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,8,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,8,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,8,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,8,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,8,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,8,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,8,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,8,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,8,1,9>,
        /* A */ &ARM::dataProcessingARM<1,8,1,10>,
        /* B */ &ARM::dataProcessingARM<1,8,1,11>,
        /* C */ &ARM::dataProcessingARM<1,8,1,12>,
        /* D */ &ARM::dataProcessingARM<1,8,1,13>,
        /* E */ &ARM::dataProcessingARM<1,8,1,14>,
        /* F */ &ARM::dataProcessingARM<1,8,1,15>,

        // 0x32X
        /* 0 */ &ARM::statusTransferARM<1,0,1>,
        /* 1 */ &ARM::statusTransferARM<1,0,1>,
        /* 2 */ &ARM::statusTransferARM<1,0,1>,
        /* 3 */ &ARM::statusTransferARM<1,0,1>,
        /* 4 */ &ARM::statusTransferARM<1,0,1>,
        /* 5 */ &ARM::statusTransferARM<1,0,1>,
        /* 6 */ &ARM::statusTransferARM<1,0,1>,
        /* 7 */ &ARM::statusTransferARM<1,0,1>,
        /* 8 */ &ARM::statusTransferARM<1,0,1>,
        /* 9 */ &ARM::statusTransferARM<1,0,1>,
        /* A */ &ARM::statusTransferARM<1,0,1>,
        /* B */ &ARM::statusTransferARM<1,0,1>,
        /* C */ &ARM::statusTransferARM<1,0,1>,
        /* D */ &ARM::statusTransferARM<1,0,1>,
        /* E */ &ARM::statusTransferARM<1,0,1>,
        /* F */ &ARM::statusTransferARM<1,0,1>,

        // 0x33X
        /* 0 */ &ARM::dataProcessingARM<1,9,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,9,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,9,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,9,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,9,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,9,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,9,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,9,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,9,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,9,1,9>,
        /* A */ &ARM::dataProcessingARM<1,9,1,10>,
        /* B */ &ARM::dataProcessingARM<1,9,1,11>,
        /* C */ &ARM::dataProcessingARM<1,9,1,12>,
        /* D */ &ARM::dataProcessingARM<1,9,1,13>,
        /* E */ &ARM::dataProcessingARM<1,9,1,14>,
        /* F */ &ARM::dataProcessingARM<1,9,1,15>,

        // 0x34X
        /* 0 */ &ARM::statusTransferARM<1,1,0>,
        /* 1 */ &ARM::statusTransferARM<1,1,0>,
        /* 2 */ &ARM::statusTransferARM<1,1,0>,
        /* 3 */ &ARM::statusTransferARM<1,1,0>,
        /* 4 */ &ARM::statusTransferARM<1,1,0>,
        /* 5 */ &ARM::statusTransferARM<1,1,0>,
        /* 6 */ &ARM::statusTransferARM<1,1,0>,
        /* 7 */ &ARM::statusTransferARM<1,1,0>,
        /* 8 */ &ARM::statusTransferARM<1,1,0>,
        /* 9 */ &ARM::statusTransferARM<1,1,0>,
        /* A */ &ARM::statusTransferARM<1,1,0>,
        /* B */ &ARM::statusTransferARM<1,1,0>,
        /* C */ &ARM::statusTransferARM<1,1,0>,
        /* D */ &ARM::statusTransferARM<1,1,0>,
        /* E */ &ARM::statusTransferARM<1,1,0>,
        /* F */ &ARM::statusTransferARM<1,1,0>,

        // 0x35X
        /* 0 */ &ARM::dataProcessingARM<1,10,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,10,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,10,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,10,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,10,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,10,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,10,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,10,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,10,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,10,1,9>,
        /* A */ &ARM::dataProcessingARM<1,10,1,10>,
        /* B */ &ARM::dataProcessingARM<1,10,1,11>,
        /* C */ &ARM::dataProcessingARM<1,10,1,12>,
        /* D */ &ARM::dataProcessingARM<1,10,1,13>,
        /* E */ &ARM::dataProcessingARM<1,10,1,14>,
        /* F */ &ARM::dataProcessingARM<1,10,1,15>,

        // 0x36X
        /* 0 */ &ARM::statusTransferARM<1,1,1>,
        /* 1 */ &ARM::statusTransferARM<1,1,1>,
        /* 2 */ &ARM::statusTransferARM<1,1,1>,
        /* 3 */ &ARM::statusTransferARM<1,1,1>,
        /* 4 */ &ARM::statusTransferARM<1,1,1>,
        /* 5 */ &ARM::statusTransferARM<1,1,1>,
        /* 6 */ &ARM::statusTransferARM<1,1,1>,
        /* 7 */ &ARM::statusTransferARM<1,1,1>,
        /* 8 */ &ARM::statusTransferARM<1,1,1>,
        /* 9 */ &ARM::statusTransferARM<1,1,1>,
        /* A */ &ARM::statusTransferARM<1,1,1>,
        /* B */ &ARM::statusTransferARM<1,1,1>,
        /* C */ &ARM::statusTransferARM<1,1,1>,
        /* D */ &ARM::statusTransferARM<1,1,1>,
        /* E */ &ARM::statusTransferARM<1,1,1>,
        /* F */ &ARM::statusTransferARM<1,1,1>,

        // 0x37X
        /* 0 */ &ARM::dataProcessingARM<1,11,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,11,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,11,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,11,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,11,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,11,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,11,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,11,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,11,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,11,1,9>,
        /* A */ &ARM::dataProcessingARM<1,11,1,10>,
        /* B */ &ARM::dataProcessingARM<1,11,1,11>,
        /* C */ &ARM::dataProcessingARM<1,11,1,12>,
        /* D */ &ARM::dataProcessingARM<1,11,1,13>,
        /* E */ &ARM::dataProcessingARM<1,11,1,14>,
        /* F */ &ARM::dataProcessingARM<1,11,1,15>,

        // 0x38X
        /* 0 */ &ARM::dataProcessingARM<1,12,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,12,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,12,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,12,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,12,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,12,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,12,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,12,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,12,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,12,0,9>,
        /* A */ &ARM::dataProcessingARM<1,12,0,10>,
        /* B */ &ARM::dataProcessingARM<1,12,0,11>,
        /* C */ &ARM::dataProcessingARM<1,12,0,12>,
        /* D */ &ARM::dataProcessingARM<1,12,0,13>,
        /* E */ &ARM::dataProcessingARM<1,12,0,14>,
        /* F */ &ARM::dataProcessingARM<1,12,0,15>,

        // 0x39X
        /* 0 */ &ARM::dataProcessingARM<1,12,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,12,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,12,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,12,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,12,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,12,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,12,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,12,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,12,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,12,1,9>,
        /* A */ &ARM::dataProcessingARM<1,12,1,10>,
        /* B */ &ARM::dataProcessingARM<1,12,1,11>,
        /* C */ &ARM::dataProcessingARM<1,12,1,12>,
        /* D */ &ARM::dataProcessingARM<1,12,1,13>,
        /* E */ &ARM::dataProcessingARM<1,12,1,14>,
        /* F */ &ARM::dataProcessingARM<1,12,1,15>,

        // 0x3AX
        /* 0 */ &ARM::dataProcessingARM<1,13,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,13,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,13,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,13,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,13,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,13,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,13,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,13,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,13,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,13,0,9>,
        /* A */ &ARM::dataProcessingARM<1,13,0,10>,
        /* B */ &ARM::dataProcessingARM<1,13,0,11>,
        /* C */ &ARM::dataProcessingARM<1,13,0,12>,
        /* D */ &ARM::dataProcessingARM<1,13,0,13>,
        /* E */ &ARM::dataProcessingARM<1,13,0,14>,
        /* F */ &ARM::dataProcessingARM<1,13,0,15>,

        // 0x3BX
        /* 0 */ &ARM::dataProcessingARM<1,13,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,13,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,13,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,13,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,13,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,13,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,13,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,13,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,13,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,13,1,9>,
        /* A */ &ARM::dataProcessingARM<1,13,1,10>,
        /* B */ &ARM::dataProcessingARM<1,13,1,11>,
        /* C */ &ARM::dataProcessingARM<1,13,1,12>,
        /* D */ &ARM::dataProcessingARM<1,13,1,13>,
        /* E */ &ARM::dataProcessingARM<1,13,1,14>,
        /* F */ &ARM::dataProcessingARM<1,13,1,15>,

        // 0x3CX
        /* 0 */ &ARM::dataProcessingARM<1,14,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,14,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,14,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,14,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,14,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,14,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,14,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,14,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,14,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,14,0,9>,
        /* A */ &ARM::dataProcessingARM<1,14,0,10>,
        /* B */ &ARM::dataProcessingARM<1,14,0,11>,
        /* C */ &ARM::dataProcessingARM<1,14,0,12>,
        /* D */ &ARM::dataProcessingARM<1,14,0,13>,
        /* E */ &ARM::dataProcessingARM<1,14,0,14>,
        /* F */ &ARM::dataProcessingARM<1,14,0,15>,

        // 0x3DX
        /* 0 */ &ARM::dataProcessingARM<1,14,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,14,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,14,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,14,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,14,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,14,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,14,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,14,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,14,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,14,1,9>,
        /* A */ &ARM::dataProcessingARM<1,14,1,10>,
        /* B */ &ARM::dataProcessingARM<1,14,1,11>,
        /* C */ &ARM::dataProcessingARM<1,14,1,12>,
        /* D */ &ARM::dataProcessingARM<1,14,1,13>,
        /* E */ &ARM::dataProcessingARM<1,14,1,14>,
        /* F */ &ARM::dataProcessingARM<1,14,1,15>,

        // 0x3EX
        /* 0 */ &ARM::dataProcessingARM<1,15,0,0>,
        /* 1 */ &ARM::dataProcessingARM<1,15,0,1>,
        /* 2 */ &ARM::dataProcessingARM<1,15,0,2>,
        /* 3 */ &ARM::dataProcessingARM<1,15,0,3>,
        /* 4 */ &ARM::dataProcessingARM<1,15,0,4>,
        /* 5 */ &ARM::dataProcessingARM<1,15,0,5>,
        /* 6 */ &ARM::dataProcessingARM<1,15,0,6>,
        /* 7 */ &ARM::dataProcessingARM<1,15,0,7>,
        /* 8 */ &ARM::dataProcessingARM<1,15,0,8>,
        /* 9 */ &ARM::dataProcessingARM<1,15,0,9>,
        /* A */ &ARM::dataProcessingARM<1,15,0,10>,
        /* B */ &ARM::dataProcessingARM<1,15,0,11>,
        /* C */ &ARM::dataProcessingARM<1,15,0,12>,
        /* D */ &ARM::dataProcessingARM<1,15,0,13>,
        /* E */ &ARM::dataProcessingARM<1,15,0,14>,
        /* F */ &ARM::dataProcessingARM<1,15,0,15>,

        // 0x3FX
        /* 0 */ &ARM::dataProcessingARM<1,15,1,0>,
        /* 1 */ &ARM::dataProcessingARM<1,15,1,1>,
        /* 2 */ &ARM::dataProcessingARM<1,15,1,2>,
        /* 3 */ &ARM::dataProcessingARM<1,15,1,3>,
        /* 4 */ &ARM::dataProcessingARM<1,15,1,4>,
        /* 5 */ &ARM::dataProcessingARM<1,15,1,5>,
        /* 6 */ &ARM::dataProcessingARM<1,15,1,6>,
        /* 7 */ &ARM::dataProcessingARM<1,15,1,7>,
        /* 8 */ &ARM::dataProcessingARM<1,15,1,8>,
        /* 9 */ &ARM::dataProcessingARM<1,15,1,9>,
        /* A */ &ARM::dataProcessingARM<1,15,1,10>,
        /* B */ &ARM::dataProcessingARM<1,15,1,11>,
        /* C */ &ARM::dataProcessingARM<1,15,1,12>,
        /* D */ &ARM::dataProcessingARM<1,15,1,13>,
        /* E */ &ARM::dataProcessingARM<1,15,1,14>,
        /* F */ &ARM::dataProcessingARM<1,15,1,15>,

        // 0x40X
        /* 0 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* A */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* B */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* C */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* D */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* E */ &ARM::singleTransferARM<1,0,0,0,0,0>,
        /* F */ &ARM::singleTransferARM<1,0,0,0,0,0>,

        // 0x41X
        /* 0 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* A */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* B */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* C */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* D */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* E */ &ARM::singleTransferARM<1,0,0,0,0,1>,
        /* F */ &ARM::singleTransferARM<1,0,0,0,0,1>,

        // 0x42X
        /* 0 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* A */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* B */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* C */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* D */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* E */ &ARM::singleTransferARM<1,0,0,0,1,0>,
        /* F */ &ARM::singleTransferARM<1,0,0,0,1,0>,

        // 0x43X
        /* 0 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* A */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* B */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* C */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* D */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* E */ &ARM::singleTransferARM<1,0,0,0,1,1>,
        /* F */ &ARM::singleTransferARM<1,0,0,0,1,1>,

        // 0x44X
        /* 0 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* A */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* B */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* C */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* D */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* E */ &ARM::singleTransferARM<1,0,0,1,0,0>,
        /* F */ &ARM::singleTransferARM<1,0,0,1,0,0>,

        // 0x45X
        /* 0 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* A */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* B */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* C */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* D */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* E */ &ARM::singleTransferARM<1,0,0,1,0,1>,
        /* F */ &ARM::singleTransferARM<1,0,0,1,0,1>,

        // 0x46X
        /* 0 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* A */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* B */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* C */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* D */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* E */ &ARM::singleTransferARM<1,0,0,1,1,0>,
        /* F */ &ARM::singleTransferARM<1,0,0,1,1,0>,

        // 0x47X
        /* 0 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* A */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* B */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* C */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* D */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* E */ &ARM::singleTransferARM<1,0,0,1,1,1>,
        /* F */ &ARM::singleTransferARM<1,0,0,1,1,1>,

        // 0x48X
        /* 0 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* A */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* B */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* C */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* D */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* E */ &ARM::singleTransferARM<1,0,1,0,0,0>,
        /* F */ &ARM::singleTransferARM<1,0,1,0,0,0>,

        // 0x49X
        /* 0 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* A */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* B */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* C */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* D */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* E */ &ARM::singleTransferARM<1,0,1,0,0,1>,
        /* F */ &ARM::singleTransferARM<1,0,1,0,0,1>,

        // 0x4AX
        /* 0 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* A */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* B */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* C */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* D */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* E */ &ARM::singleTransferARM<1,0,1,0,1,0>,
        /* F */ &ARM::singleTransferARM<1,0,1,0,1,0>,

        // 0x4BX
        /* 0 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* A */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* B */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* C */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* D */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* E */ &ARM::singleTransferARM<1,0,1,0,1,1>,
        /* F */ &ARM::singleTransferARM<1,0,1,0,1,1>,

        // 0x4CX
        /* 0 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* A */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* B */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* C */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* D */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* E */ &ARM::singleTransferARM<1,0,1,1,0,0>,
        /* F */ &ARM::singleTransferARM<1,0,1,1,0,0>,

        // 0x4DX
        /* 0 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* A */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* B */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* C */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* D */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* E */ &ARM::singleTransferARM<1,0,1,1,0,1>,
        /* F */ &ARM::singleTransferARM<1,0,1,1,0,1>,

        // 0x4EX
        /* 0 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* A */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* B */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* C */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* D */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* E */ &ARM::singleTransferARM<1,0,1,1,1,0>,
        /* F */ &ARM::singleTransferARM<1,0,1,1,1,0>,

        // 0x4FX
        /* 0 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* A */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* B */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* C */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* D */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* E */ &ARM::singleTransferARM<1,0,1,1,1,1>,
        /* F */ &ARM::singleTransferARM<1,0,1,1,1,1>,

        // 0x50X
        /* 0 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* A */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* B */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* C */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* D */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* E */ &ARM::singleTransferARM<1,1,0,0,0,0>,
        /* F */ &ARM::singleTransferARM<1,1,0,0,0,0>,

        // 0x51X
        /* 0 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* A */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* B */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* C */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* D */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* E */ &ARM::singleTransferARM<1,1,0,0,0,1>,
        /* F */ &ARM::singleTransferARM<1,1,0,0,0,1>,

        // 0x52X
        /* 0 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* A */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* B */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* C */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* D */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* E */ &ARM::singleTransferARM<1,1,0,0,1,0>,
        /* F */ &ARM::singleTransferARM<1,1,0,0,1,0>,

        // 0x53X
        /* 0 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* A */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* B */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* C */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* D */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* E */ &ARM::singleTransferARM<1,1,0,0,1,1>,
        /* F */ &ARM::singleTransferARM<1,1,0,0,1,1>,

        // 0x54X
        /* 0 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* A */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* B */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* C */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* D */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* E */ &ARM::singleTransferARM<1,1,0,1,0,0>,
        /* F */ &ARM::singleTransferARM<1,1,0,1,0,0>,

        // 0x55X
        /* 0 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* A */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* B */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* C */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* D */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* E */ &ARM::singleTransferARM<1,1,0,1,0,1>,
        /* F */ &ARM::singleTransferARM<1,1,0,1,0,1>,

        // 0x56X
        /* 0 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* A */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* B */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* C */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* D */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* E */ &ARM::singleTransferARM<1,1,0,1,1,0>,
        /* F */ &ARM::singleTransferARM<1,1,0,1,1,0>,

        // 0x57X
        /* 0 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* A */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* B */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* C */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* D */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* E */ &ARM::singleTransferARM<1,1,0,1,1,1>,
        /* F */ &ARM::singleTransferARM<1,1,0,1,1,1>,

        // 0x58X
        /* 0 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* A */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* B */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* C */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* D */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* E */ &ARM::singleTransferARM<1,1,1,0,0,0>,
        /* F */ &ARM::singleTransferARM<1,1,1,0,0,0>,

        // 0x59X
        /* 0 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* A */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* B */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* C */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* D */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* E */ &ARM::singleTransferARM<1,1,1,0,0,1>,
        /* F */ &ARM::singleTransferARM<1,1,1,0,0,1>,

        // 0x5AX
        /* 0 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* A */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* B */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* C */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* D */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* E */ &ARM::singleTransferARM<1,1,1,0,1,0>,
        /* F */ &ARM::singleTransferARM<1,1,1,0,1,0>,

        // 0x5BX
        /* 0 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* A */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* B */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* C */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* D */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* E */ &ARM::singleTransferARM<1,1,1,0,1,1>,
        /* F */ &ARM::singleTransferARM<1,1,1,0,1,1>,

        // 0x5CX
        /* 0 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* A */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* B */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* C */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* D */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* E */ &ARM::singleTransferARM<1,1,1,1,0,0>,
        /* F */ &ARM::singleTransferARM<1,1,1,1,0,0>,

        // 0x5DX
        /* 0 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* A */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* B */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* C */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* D */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* E */ &ARM::singleTransferARM<1,1,1,1,0,1>,
        /* F */ &ARM::singleTransferARM<1,1,1,1,0,1>,

        // 0x5EX
        /* 0 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* A */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* B */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* C */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* D */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* E */ &ARM::singleTransferARM<1,1,1,1,1,0>,
        /* F */ &ARM::singleTransferARM<1,1,1,1,1,0>,

        // 0x5FX
        /* 0 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 1 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 2 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 3 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 4 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 5 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 6 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 7 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 8 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* 9 */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* A */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* B */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* C */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* D */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* E */ &ARM::singleTransferARM<1,1,1,1,1,1>,
        /* F */ &ARM::singleTransferARM<1,1,1,1,1,1>,

        // 0x60X
        /* 0 */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,0,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x61X
        /* 0 */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,0,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x62X
        /* 0 */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,0,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x63X
        /* 0 */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,0,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x64X
        /* 0 */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,1,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x65X
        /* 0 */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,1,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x66X
        /* 0 */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,1,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x67X
        /* 0 */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,0,1,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x68X
        /* 0 */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,0,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x69X
        /* 0 */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,0,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6AX
        /* 0 */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,0,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6BX
        /* 0 */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,0,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6CX
        /* 0 */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,1,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6DX
        /* 0 */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,1,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6EX
        /* 0 */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,1,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x6FX
        /* 0 */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,0,1,1,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x70X
        /* 0 */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,0,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x71X
        /* 0 */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,0,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x72X
        /* 0 */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,0,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x73X
        /* 0 */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,0,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x74X
        /* 0 */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,1,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x75X
        /* 0 */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,1,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x76X
        /* 0 */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,1,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x77X
        /* 0 */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,0,1,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x78X
        /* 0 */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,0,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x79X
        /* 0 */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,0,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7AX
        /* 0 */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,0,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7BX
        /* 0 */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,0,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7CX
        /* 0 */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,1,0,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7DX
        /* 0 */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,1,0,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7EX
        /* 0 */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,1,1,0>,
        /* F */ &ARM::undefinedInstARM,

        // 0x7FX
        /* 0 */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::singleTransferARM<0,1,1,1,1,1>,
        /* F */ &ARM::undefinedInstARM,

        // 0x80X
        /* 0 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 1 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 2 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 3 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 4 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 5 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 6 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 7 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 8 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* 9 */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* A */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* B */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* C */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* D */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* E */ &ARM::blockTransferARM<0,0,0,0,0>,
        /* F */ &ARM::blockTransferARM<0,0,0,0,0>,

        // 0x81X
        /* 0 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 1 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 2 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 3 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 4 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 5 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 6 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 7 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 8 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* 9 */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* A */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* B */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* C */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* D */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* E */ &ARM::blockTransferARM<0,0,0,0,1>,
        /* F */ &ARM::blockTransferARM<0,0,0,0,1>,

        // 0x82X
        /* 0 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 1 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 2 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 3 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 4 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 5 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 6 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 7 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 8 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* 9 */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* A */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* B */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* C */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* D */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* E */ &ARM::blockTransferARM<0,0,0,1,0>,
        /* F */ &ARM::blockTransferARM<0,0,0,1,0>,

        // 0x83X
        /* 0 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 1 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 2 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 3 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 4 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 5 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 6 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 7 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 8 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* 9 */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* A */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* B */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* C */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* D */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* E */ &ARM::blockTransferARM<0,0,0,1,1>,
        /* F */ &ARM::blockTransferARM<0,0,0,1,1>,

        // 0x84X
        /* 0 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 1 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 2 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 3 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 4 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 5 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 6 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 7 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 8 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* 9 */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* A */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* B */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* C */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* D */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* E */ &ARM::blockTransferARM<0,0,1,0,0>,
        /* F */ &ARM::blockTransferARM<0,0,1,0,0>,

        // 0x85X
        /* 0 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 1 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 2 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 3 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 4 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 5 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 6 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 7 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 8 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* 9 */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* A */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* B */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* C */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* D */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* E */ &ARM::blockTransferARM<0,0,1,0,1>,
        /* F */ &ARM::blockTransferARM<0,0,1,0,1>,

        // 0x86X
        /* 0 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 1 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 2 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 3 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 4 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 5 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 6 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 7 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 8 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* 9 */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* A */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* B */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* C */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* D */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* E */ &ARM::blockTransferARM<0,0,1,1,0>,
        /* F */ &ARM::blockTransferARM<0,0,1,1,0>,

        // 0x87X
        /* 0 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 1 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 2 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 3 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 4 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 5 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 6 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 7 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 8 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* 9 */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* A */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* B */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* C */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* D */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* E */ &ARM::blockTransferARM<0,0,1,1,1>,
        /* F */ &ARM::blockTransferARM<0,0,1,1,1>,

        // 0x88X
        /* 0 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 1 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 2 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 3 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 4 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 5 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 6 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 7 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 8 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* 9 */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* A */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* B */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* C */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* D */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* E */ &ARM::blockTransferARM<0,1,0,0,0>,
        /* F */ &ARM::blockTransferARM<0,1,0,0,0>,

        // 0x89X
        /* 0 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 1 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 2 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 3 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 4 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 5 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 6 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 7 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 8 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* 9 */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* A */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* B */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* C */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* D */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* E */ &ARM::blockTransferARM<0,1,0,0,1>,
        /* F */ &ARM::blockTransferARM<0,1,0,0,1>,

        // 0x8AX
        /* 0 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 1 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 2 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 3 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 4 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 5 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 6 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 7 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 8 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* 9 */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* A */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* B */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* C */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* D */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* E */ &ARM::blockTransferARM<0,1,0,1,0>,
        /* F */ &ARM::blockTransferARM<0,1,0,1,0>,

        // 0x8BX
        /* 0 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 1 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 2 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 3 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 4 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 5 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 6 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 7 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 8 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* 9 */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* A */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* B */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* C */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* D */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* E */ &ARM::blockTransferARM<0,1,0,1,1>,
        /* F */ &ARM::blockTransferARM<0,1,0,1,1>,

        // 0x8CX
        /* 0 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 1 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 2 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 3 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 4 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 5 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 6 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 7 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 8 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* 9 */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* A */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* B */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* C */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* D */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* E */ &ARM::blockTransferARM<0,1,1,0,0>,
        /* F */ &ARM::blockTransferARM<0,1,1,0,0>,

        // 0x8DX
        /* 0 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 1 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 2 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 3 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 4 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 5 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 6 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 7 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 8 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* 9 */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* A */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* B */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* C */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* D */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* E */ &ARM::blockTransferARM<0,1,1,0,1>,
        /* F */ &ARM::blockTransferARM<0,1,1,0,1>,

        // 0x8EX
        /* 0 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 1 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 2 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 3 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 4 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 5 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 6 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 7 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 8 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* 9 */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* A */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* B */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* C */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* D */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* E */ &ARM::blockTransferARM<0,1,1,1,0>,
        /* F */ &ARM::blockTransferARM<0,1,1,1,0>,

        // 0x8FX
        /* 0 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 1 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 2 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 3 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 4 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 5 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 6 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 7 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 8 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* 9 */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* A */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* B */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* C */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* D */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* E */ &ARM::blockTransferARM<0,1,1,1,1>,
        /* F */ &ARM::blockTransferARM<0,1,1,1,1>,

        // 0x90X
        /* 0 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 1 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 2 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 3 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 4 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 5 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 6 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 7 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 8 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* 9 */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* A */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* B */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* C */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* D */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* E */ &ARM::blockTransferARM<1,0,0,0,0>,
        /* F */ &ARM::blockTransferARM<1,0,0,0,0>,

        // 0x91X
        /* 0 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 1 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 2 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 3 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 4 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 5 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 6 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 7 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 8 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* 9 */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* A */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* B */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* C */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* D */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* E */ &ARM::blockTransferARM<1,0,0,0,1>,
        /* F */ &ARM::blockTransferARM<1,0,0,0,1>,

        // 0x92X
        /* 0 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 1 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 2 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 3 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 4 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 5 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 6 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 7 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 8 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* 9 */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* A */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* B */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* C */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* D */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* E */ &ARM::blockTransferARM<1,0,0,1,0>,
        /* F */ &ARM::blockTransferARM<1,0,0,1,0>,

        // 0x93X
        /* 0 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 1 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 2 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 3 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 4 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 5 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 6 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 7 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 8 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* 9 */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* A */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* B */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* C */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* D */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* E */ &ARM::blockTransferARM<1,0,0,1,1>,
        /* F */ &ARM::blockTransferARM<1,0,0,1,1>,

        // 0x94X
        /* 0 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 1 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 2 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 3 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 4 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 5 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 6 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 7 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 8 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* 9 */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* A */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* B */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* C */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* D */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* E */ &ARM::blockTransferARM<1,0,1,0,0>,
        /* F */ &ARM::blockTransferARM<1,0,1,0,0>,

        // 0x95X
        /* 0 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 1 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 2 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 3 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 4 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 5 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 6 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 7 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 8 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* 9 */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* A */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* B */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* C */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* D */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* E */ &ARM::blockTransferARM<1,0,1,0,1>,
        /* F */ &ARM::blockTransferARM<1,0,1,0,1>,

        // 0x96X
        /* 0 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 1 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 2 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 3 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 4 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 5 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 6 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 7 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 8 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* 9 */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* A */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* B */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* C */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* D */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* E */ &ARM::blockTransferARM<1,0,1,1,0>,
        /* F */ &ARM::blockTransferARM<1,0,1,1,0>,

        // 0x97X
        /* 0 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 1 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 2 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 3 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 4 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 5 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 6 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 7 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 8 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* 9 */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* A */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* B */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* C */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* D */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* E */ &ARM::blockTransferARM<1,0,1,1,1>,
        /* F */ &ARM::blockTransferARM<1,0,1,1,1>,

        // 0x98X
        /* 0 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 1 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 2 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 3 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 4 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 5 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 6 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 7 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 8 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* 9 */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* A */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* B */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* C */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* D */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* E */ &ARM::blockTransferARM<1,1,0,0,0>,
        /* F */ &ARM::blockTransferARM<1,1,0,0,0>,

        // 0x99X
        /* 0 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 1 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 2 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 3 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 4 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 5 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 6 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 7 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 8 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* 9 */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* A */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* B */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* C */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* D */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* E */ &ARM::blockTransferARM<1,1,0,0,1>,
        /* F */ &ARM::blockTransferARM<1,1,0,0,1>,

        // 0x9AX
        /* 0 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 1 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 2 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 3 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 4 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 5 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 6 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 7 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 8 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* 9 */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* A */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* B */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* C */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* D */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* E */ &ARM::blockTransferARM<1,1,0,1,0>,
        /* F */ &ARM::blockTransferARM<1,1,0,1,0>,

        // 0x9BX
        /* 0 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 1 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 2 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 3 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 4 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 5 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 6 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 7 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 8 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* 9 */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* A */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* B */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* C */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* D */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* E */ &ARM::blockTransferARM<1,1,0,1,1>,
        /* F */ &ARM::blockTransferARM<1,1,0,1,1>,

        // 0x9CX
        /* 0 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 1 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 2 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 3 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 4 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 5 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 6 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 7 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 8 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* 9 */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* A */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* B */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* C */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* D */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* E */ &ARM::blockTransferARM<1,1,1,0,0>,
        /* F */ &ARM::blockTransferARM<1,1,1,0,0>,

        // 0x9DX
        /* 0 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 1 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 2 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 3 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 4 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 5 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 6 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 7 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 8 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* 9 */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* A */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* B */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* C */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* D */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* E */ &ARM::blockTransferARM<1,1,1,0,1>,
        /* F */ &ARM::blockTransferARM<1,1,1,0,1>,

        // 0x9EX
        /* 0 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 1 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 2 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 3 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 4 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 5 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 6 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 7 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 8 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* 9 */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* A */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* B */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* C */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* D */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* E */ &ARM::blockTransferARM<1,1,1,1,0>,
        /* F */ &ARM::blockTransferARM<1,1,1,1,0>,

        // 0x9FX
        /* 0 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 1 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 2 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 3 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 4 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 5 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 6 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 7 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 8 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* 9 */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* A */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* B */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* C */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* D */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* E */ &ARM::blockTransferARM<1,1,1,1,1>,
        /* F */ &ARM::blockTransferARM<1,1,1,1,1>,

        // 0xA0X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA1X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA2X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA3X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA4X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA5X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA6X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA7X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA8X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xA9X
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xAAX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xABX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xACX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xADX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xAEX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xAFX
        /* 0 */ &ARM::branchARM<0>,
        /* 1 */ &ARM::branchARM<0>,
        /* 2 */ &ARM::branchARM<0>,
        /* 3 */ &ARM::branchARM<0>,
        /* 4 */ &ARM::branchARM<0>,
        /* 5 */ &ARM::branchARM<0>,
        /* 6 */ &ARM::branchARM<0>,
        /* 7 */ &ARM::branchARM<0>,
        /* 8 */ &ARM::branchARM<0>,
        /* 9 */ &ARM::branchARM<0>,
        /* A */ &ARM::branchARM<0>,
        /* B */ &ARM::branchARM<0>,
        /* C */ &ARM::branchARM<0>,
        /* D */ &ARM::branchARM<0>,
        /* E */ &ARM::branchARM<0>,
        /* F */ &ARM::branchARM<0>,

        // 0xB0X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB1X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB2X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB3X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB4X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB5X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB6X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB7X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB8X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xB9X
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBAX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBBX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBCX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBDX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBEX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xBFX
        /* 0 */ &ARM::branchARM<1>,
        /* 1 */ &ARM::branchARM<1>,
        /* 2 */ &ARM::branchARM<1>,
        /* 3 */ &ARM::branchARM<1>,
        /* 4 */ &ARM::branchARM<1>,
        /* 5 */ &ARM::branchARM<1>,
        /* 6 */ &ARM::branchARM<1>,
        /* 7 */ &ARM::branchARM<1>,
        /* 8 */ &ARM::branchARM<1>,
        /* 9 */ &ARM::branchARM<1>,
        /* A */ &ARM::branchARM<1>,
        /* B */ &ARM::branchARM<1>,
        /* C */ &ARM::branchARM<1>,
        /* D */ &ARM::branchARM<1>,
        /* E */ &ARM::branchARM<1>,
        /* F */ &ARM::branchARM<1>,

        // 0xC0X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC1X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC2X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC3X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC4X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC5X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC6X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC7X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC8X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xC9X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCAX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCBX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCCX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCDX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCEX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xCFX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD0X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD1X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD2X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD3X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD4X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD5X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD6X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD7X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD8X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xD9X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDAX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDBX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDCX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDDX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDEX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xDFX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE0X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE1X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE2X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE3X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE4X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE5X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE6X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE7X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE8X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xE9X
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xEAX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xEBX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xECX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xEDX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xEEX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xEFX
        /* 0 */ &ARM::undefinedInstARM,
        /* 1 */ &ARM::undefinedInstARM,
        /* 2 */ &ARM::undefinedInstARM,
        /* 3 */ &ARM::undefinedInstARM,
        /* 4 */ &ARM::undefinedInstARM,
        /* 5 */ &ARM::undefinedInstARM,
        /* 6 */ &ARM::undefinedInstARM,
        /* 7 */ &ARM::undefinedInstARM,
        /* 8 */ &ARM::undefinedInstARM,
        /* 9 */ &ARM::undefinedInstARM,
        /* A */ &ARM::undefinedInstARM,
        /* B */ &ARM::undefinedInstARM,
        /* C */ &ARM::undefinedInstARM,
        /* D */ &ARM::undefinedInstARM,
        /* E */ &ARM::undefinedInstARM,
        /* F */ &ARM::undefinedInstARM,

        // 0xF0X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF1X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF2X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF3X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF4X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF5X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF6X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF7X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF8X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xF9X
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFAX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFBX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFCX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFDX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFEX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM,

        // 0xFFX
        /* 0 */ &ARM::swiARM,
        /* 1 */ &ARM::swiARM,
        /* 2 */ &ARM::swiARM,
        /* 3 */ &ARM::swiARM,
        /* 4 */ &ARM::swiARM,
        /* 5 */ &ARM::swiARM,
        /* 6 */ &ARM::swiARM,
        /* 7 */ &ARM::swiARM,
        /* 8 */ &ARM::swiARM,
        /* 9 */ &ARM::swiARM,
        /* A */ &ARM::swiARM,
        /* B */ &ARM::swiARM,
        /* C */ &ARM::swiARM,
        /* D */ &ARM::swiARM,
        /* E */ &ARM::swiARM,
        /* F */ &ARM::swiARM
    };
}
