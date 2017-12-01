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

inline auto ARM::opDataProc(u32 result, bool set_nz, bool set_c, bool carry) -> u32 {
    if (set_nz && set_c) {
        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG | MASK_CFLAG);

        if (result & (1 << 31)) ctx.cpsr |= MASK_NFLAG;
        if (result == 0)        ctx.cpsr |= MASK_ZFLAG;
        if (carry)              ctx.cpsr |= MASK_CFLAG;
    }
    else if (set_nz && !set_c) {
        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG);

        if (result & (1 << 31)) ctx.cpsr |= MASK_NFLAG;
        if (result == 0)        ctx.cpsr |= MASK_ZFLAG;
    }
    return result;
}

inline auto ARM::opADD(u32 op1, u32 op2, bool set_flags) -> u32 {
    if (set_flags) {
        u64 result64 = (u64)op1 + (u64)op2;
        u32 result32 = (u32)result64;

        u32 overflow = (~( op1 ^ op2) & ((op1 + op2) ^ op2)) >> 31;

        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG | MASK_CFLAG | MASK_VFLAG);

        if (result32 & (1 << 31))      ctx.cpsr |= MASK_NFLAG;
        if (result32 == 0)             ctx.cpsr |= MASK_ZFLAG;
        if (result64 & 0x100000000ULL) ctx.cpsr |= MASK_CFLAG;
        if (overflow)                  ctx.cpsr |= MASK_VFLAG;

        return result32;
    }
    return op1 + op2;
}

inline auto ARM::opADC(u32 op1, u32 op2, u32 op3, bool set_flags) -> u32 {
    if (set_flags) {
        u64 result64 = (u64)op1 + (u64)op2 + (u64)op3;
        u32 result32 = (u32)result64;

        // TODO: confirm the correctness of this logic
        u32 overflow = ((~( op1        ^ op2) & ((op1 + op2)       ^ op2))  ^
                        (~((op1 + op2) ^ op3) & ((op1 + op2 + op3) ^ op3))) >> 31;

        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG | MASK_CFLAG | MASK_VFLAG);

        if (result32 & (1 << 31))      ctx.cpsr |= MASK_NFLAG;
        if (result32 == 0)             ctx.cpsr |= MASK_ZFLAG;
        if (result64 & 0x100000000ULL) ctx.cpsr |= MASK_CFLAG;
        if (overflow)                  ctx.cpsr |= MASK_VFLAG;

        return result32;
    }
    return op1 + op2 + op3;
}

inline auto ARM::opSUB(u32 op1, u32 op2, bool set_flags) -> u32 {
    if (set_flags) {
        u32 result   =   op1 - op2;
        u32 overflow = ((op1 ^ op2) & ~(result ^ op2)) >> 31;

        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG | MASK_CFLAG | MASK_VFLAG);

        if (result & (1 << 31)) ctx.cpsr |= MASK_NFLAG;
        if (result == 0)        ctx.cpsr |= MASK_ZFLAG;
        if (op1 >= op2)         ctx.cpsr |= MASK_CFLAG;
        if (overflow)           ctx.cpsr |= MASK_VFLAG;

        return result;
    }
    return op1 - op2;
}

inline auto ARM::opSBC(u32 op1, u32 op2, u32 carry, bool set_flags) -> u32 {
    if (set_flags) {
        u32 result_1 = op1      - op2; // interim result
        u32 result_2 = result_1 - carry;

        u32 overflow = (((op1 ^ op2) & ~(result_1 ^ op2)) ^
                       (result_1     & ~ result_2      )) >> 31;

        ctx.cpsr &= ~(MASK_NFLAG | MASK_ZFLAG | MASK_CFLAG | MASK_VFLAG);

        if (result_2 & (1 << 31)) ctx.cpsr |= MASK_NFLAG;
        if (result_2 == 0)        ctx.cpsr |= MASK_ZFLAG;
        if (overflow)             ctx.cpsr |= MASK_VFLAG;

        if ((op1 >= op2) && (result_1 >= carry)) ctx.cpsr |= MASK_CFLAG;

        return result_2;
    }
    return op1 - op2 - carry;
}

inline void ARM::shiftLSL(u32& operand, u32 amount, bool& carry) {
    if (amount == 0) return;

#if defined(__i386__) || defined(__x86_64__)
    if (UNLIKELY(amount >= 32)) {
        if (amount > 32) {
            carry = false;
        }
        else {
            carry = operand & 1;
        }
        operand = 0;
        return;
    }
#endif
    carry     = (operand << (amount - 1)) & (1 << 31);
    operand <<= amount;
}

inline void ARM::shiftLSR(u32& operand, u32 amount, bool& carry, bool immediate) {
    // LSR #0 equals to LSR #32
    if (immediate && amount == 0) amount = 32;

#if defined(__i386__) || defined(__x86_64__)
    if (UNLIKELY(amount >= 32)) {
        if (amount > 32) {
            carry = false;
        }
        else {
            carry = operand & (1 << 31);
        }
        operand = 0;
        return;
    }
#endif
    carry     = (operand >> (amount - 1)) & 1;
    operand >>= amount;
}

inline void ARM::shiftASR(u32& operand, u32 amount, bool& carry, bool immediate) {
    u32 sign_bit = operand & 0x80000000;

    // ASR #0 equals to ASR #32
    if (immediate && amount == 0) amount = 32;

    for (u32 i = 0; i < amount; i++) {
        carry   = operand & 1;
        operand = (operand >> 1) | sign_bit;
    }
}

inline void ARM::shiftROR(u32& operand, u32 amount, bool& carry, bool immediate) {
    // ROR #0 equals to RRX #1
    if (amount != 0 || !immediate) {
        for (u32 i = 1; i <= amount; i++) {
            u32 high_bit = (operand & 1) << 31;

            operand = (operand >> 1) | high_bit;
            carry   = high_bit;
        }
    } else {
        bool old_carry = carry;

        carry   = operand & 1;
        operand = (operand >> 1) | (old_carry ? 0x80000000 : 0);
    }
}

inline void ARM::applyShift(int shift, u32& operand, u32 amount, bool& carry, bool immediate) {
    switch (shift) {
    case 0: shiftLSL(operand, amount, carry);            return;
    case 1: shiftLSR(operand, amount, carry, immediate); return;
    case 2: shiftASR(operand, amount, carry, immediate); return;
    case 3: shiftROR(operand, amount, carry, immediate); return;
    }
}
