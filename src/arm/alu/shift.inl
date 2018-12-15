/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* TODO: Check if shift amount should be masked with 0xFF */

inline void ARM7::DoShift(int opcode, std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
    switch (opcode) {
        case 0: LSL(operand, amount, carry); break;
        case 1: LSR(operand, amount, carry, immediate); break;
        case 2: ASR(operand, amount, carry, immediate); break;
        case 3: ROR(operand, amount, carry, immediate); break;
    }
}

inline void ARM7::LSL(std::uint32_t& operand, std::uint32_t amount, int& carry) {
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

inline void ARM7::LSR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
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

inline void ARM7::ASR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
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

inline void ARM7::ROR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
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
