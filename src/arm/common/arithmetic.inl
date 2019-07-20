/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#ifndef ARM_INCLUDE_GUARD
#error "This file cannot be included regularely!"
#endif

void SetNZ(std::uint32_t value) {
    state.cpsr.f.n = value >> 31;
    state.cpsr.f.z = (value == 0);
}

std::uint32_t ADD(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    if (set_flags) {
        std::uint64_t result64 = (std::uint64_t)op1 + (std::uint64_t)op2;
        std::uint32_t result32 = (std::uint32_t)result64;

        SetNZ(result32);
        state.cpsr.f.c = result64 >> 32;
        state.cpsr.f.v = (~(op1 ^ op2) & (result32 ^ op2)) >> 31;

        return result32;
    } else {
        return op1 + op2;
    }
}

std::uint32_t ADC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t op3 = state.cpsr.f.c;

    if (set_flags) {
        std::uint64_t result64 = (std::uint64_t)op1 + (std::uint64_t)op2 + (std::uint64_t)op3;
        std::uint32_t result32 = (std::uint32_t)result64;

        SetNZ(result32);
        state.cpsr.f.c = result64 >> 32;
        state.cpsr.f.v = ((~(op1 ^ op2) & ((op1 + op2) ^ op2)) ^
                          (~((op1 + op2) ^ op3) & (result32 ^ op3))) >> 31;

        return result32;
    } else {
        return op1 + op2 + op3;
    }
}

std::uint32_t SUB(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t result = op1 - op2;

    if (set_flags) {
        SetNZ(result);
        state.cpsr.f.c = op1 >= op2;
        state.cpsr.f.v = ((op1 ^ op2) & ~(result ^ op2)) >> 31;
    }

    return result;
}

std::uint32_t SBC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t op3 = (state.cpsr.f.c) ^ 1;

    if (set_flags) {
        std::uint32_t result1 = op1 - op2;
        std::uint32_t result2 = result1 - op3;

        SetNZ(result2);
        state.cpsr.f.c = (op1 >= op2) && (result1 >= op3);
        state.cpsr.f.v = (((op1 ^ op2) & ~(result1 ^ op2)) ^ (result1 & ~result2)) >> 31;

        return result2;
    } else {
        return op1 - op2 - op3;
    }
}

/* CHECKME: should shift amount be masked by 0xFF? */

void DoShift(int opcode, std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
    switch (opcode) {
        case 0: LSL(operand, amount, carry); break;
        case 1: LSR(operand, amount, carry, immediate); break;
        case 2: ASR(operand, amount, carry, immediate); break;
        case 3: ROR(operand, amount, carry, immediate); break;
    }
}

void LSL(std::uint32_t& operand, std::uint32_t amount, int& carry) {
    if (amount == 0) return;

#if (defined(__i386__) || defined(__x86_64__))
    if (amount >= 32) {
        if (amount > 32) {
            carry = 0;
        } else {
            carry = operand & 1;
        }
        operand = 0;
        return;
    }
#endif
    carry = (operand << (amount - 1)) >> 31;
    operand <<= amount;
}

void LSR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
    // LSR #0 equals to LSR #32
    if (immediate && amount == 0) amount = 32;

#if (defined(__i386__) || defined(__x86_64__))
    if (amount >= 32) {
        if (amount > 32) {
            carry = 0;
        } else {
            carry = operand >> 31;
        }
        operand = 0;
        return;
    }
#endif
    carry = (operand >> (amount - 1)) & 1;
    operand >>= amount;
}

void ASR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
    std::uint32_t sign_bit = operand & 0x80000000;

    // ASR #0 equals to ASR #32
    if (immediate && amount == 0) amount = 32;

    int msb = operand >> 31;

#if (defined(__i386__) || defined(__x86_64__))
    if (amount >= 32) {
        carry = msb;
        operand = 0xFFFFFFFF * msb;
        return;
    }
    if (amount == 0) return;
#endif
    carry = msb;
    operand = (operand >> amount) | ((0xFFFFFFFF * msb) << (32 - amount));
    // for (std::uint32_t i = 0; i < amount; i++) {
    //     carry   = operand & 1;
    //     operand = (operand >> 1) | sign_bit;
    // }
}

void ROR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
    std::uint32_t lsb;

    // ROR #0 equals to RRX #1
    if (amount != 0 || !immediate) {
        if (amount == 0) return;

        amount %= 32;
        operand = (operand >> (amount - 1)) | (operand << (32 - amount + 1));
        carry = operand & 1;
        operand = (operand >> 1) | (operand << 31); 
        // for (std::uint32_t i = 1; i <= amount; i++) {
        //     lsb = operand & 1;
        //     operand = (operand >> 1) | (lsb << 31);
        //     carry = lsb;
        // }
    } else {
        lsb = operand & 1;
        operand = (operand >> 1) | (carry << 31);
        carry = lsb;
    }
}
