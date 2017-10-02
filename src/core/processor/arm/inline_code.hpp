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

#define Z_FLAG (ctx.cpsr & MASK_ZFLAG)
#define C_FLAG (ctx.cpsr & MASK_CFLAG)
#define N_FLAG (ctx.cpsr & MASK_NFLAG)
#define V_FLAG (ctx.cpsr & MASK_VFLAG)

#define XOR_BIT_31(a, b) (((a) ^ (b)) >> 31)

// MEH: place this somewhere else. It should be inline though :/
inline void ARM::step() {
    auto& pipe = ctx.pipe;

    if (ctx.cpsr & MASK_THUMB) {
        ctx.r15 &= ~1;
        executeThumb(pipe.opcode[pipe.index]);
    } else {
        ctx.r15 &= ~3;

        if (pipe.index == 0) {
            pipe.opcode[2] = busRead32(ctx.r15, M_SEQ);
        } else {
            pipe.opcode[pipe.index - 1] = busRead32(ctx.r15, M_SEQ);
        }

        executeARM(pipe.opcode[pipe.index]);
    }
}

inline bool ARM::checkCondition(Condition condition) {
    if (condition == COND_AL) {
        return true;
    }

    switch (condition) {
        case COND_EQ: return  Z_FLAG;
        case COND_NE: return !Z_FLAG;
        case COND_CS: return  C_FLAG;
        case COND_CC: return !C_FLAG;
        case COND_MI: return  N_FLAG;
        case COND_PL: return !N_FLAG;
        case COND_VS: return  V_FLAG;
        case COND_VC: return !V_FLAG;
        case COND_HI: return  C_FLAG && !Z_FLAG;
        case COND_LS: return !C_FLAG ||  Z_FLAG;
        case COND_GE: return  N_FLAG ==  V_FLAG;
        case COND_LT: return  N_FLAG !=  V_FLAG;
        case COND_GT: return !Z_FLAG && (N_FLAG == V_FLAG);
        case COND_LE: return  Z_FLAG || (N_FLAG != V_FLAG);
        case COND_AL: return true;
        case COND_NV: return false;
    }

    return false;
}

inline void ARM::updateSignFlag(u32 result) {
    if (result >> 31) {
        ctx.cpsr |= MASK_NFLAG;
    } else {
        ctx.cpsr &= ~MASK_NFLAG;
    }
}

inline void ARM::updateZeroFlag(u64 result) {
    if (result == 0) {
        ctx.cpsr |= MASK_ZFLAG;
    } else {
        ctx.cpsr &= ~MASK_ZFLAG;
    }
}

inline void ARM::updateCarryFlag(bool carry) {
    if (carry) {
        ctx.cpsr |= MASK_CFLAG;
    } else {
        ctx.cpsr &= ~MASK_CFLAG;
    }
}

inline void ARM::updateOverflowFlagAdd(u32 result, u32 operand1, u32 operand2) {
    bool overflow = !XOR_BIT_31(operand1, operand2) && XOR_BIT_31(result, operand2);

    if (overflow) {
        ctx.cpsr |= MASK_VFLAG;
    } else {
        ctx.cpsr &= ~MASK_VFLAG;
    }
}

inline void ARM::updateOverflowFlagSub(u32 result, u32 operand1, u32 operand2) {
    bool overflow = XOR_BIT_31(operand1, operand2) && !XOR_BIT_31(result, operand2);

    if (overflow) {
        ctx.cpsr |= MASK_VFLAG;
    } else {
        ctx.cpsr &= ~MASK_VFLAG;
    }
}

inline void ARM::refillPipeline() {
    if (ctx.cpsr & MASK_THUMB) {
        ctx.pipe.opcode[0] = busRead16(ctx.r15,     M_NONSEQ);
        ctx.pipe.opcode[1] = busRead16(ctx.r15 + 2, M_SEQ);
        ctx.r15 += 4;
    } else {
        ctx.pipe.opcode[0] = busRead32(ctx.r15,     M_NONSEQ);
        ctx.pipe.opcode[1] = busRead32(ctx.r15 + 4, M_SEQ);
        ctx.r15 += 8;
    }
    ctx.pipe.index = 0;
}
