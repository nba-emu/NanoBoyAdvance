/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline std::uint32_t ARM7::ADD(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
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

inline std::uint32_t ARM7::ADC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t op3 = state.cpsr.f.c;

    if (set_flags) {
        std::uint64_t result64 = (std::uint64_t)op1 + (std::uint64_t)op2 + (std::uint64_t)op3;
        std::uint32_t result32 = (std::uint32_t)result64;

        /* TODO: confirm that overflow logic is correct. */
        SetNZ(result32);
        state.cpsr.f.c = result64 >> 32;
        state.cpsr.f.v = ((~(op1 ^ op2) & ((op1 + op2) ^ op2)) ^
                          (~((op1 + op2) ^ op3) & (result32 ^ op3))) >> 31;

        return result32;
    } else {
        return op1 + op2 + op3;
    }
}

inline std::uint32_t ARM7::SUB(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t result = op1 - op2;

    if (set_flags) {
        SetNZ(result);
        state.cpsr.f.c = op1 >= op2;
        state.cpsr.f.v = ((op1 ^ op2) & ~(result ^ op2)) >> 31;
    }

    return result;
}

inline std::uint32_t ARM7::SBC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
    std::uint32_t op3 = (state.cpsr.f.c) ^ 1;

    if (set_flags) {
        std::uint32_t result1 = op1 - op2;
        std::uint32_t result2 = result1 - op3;

        /* TODO: confirm that overflow logic is correct. */
        SetNZ(result2);
        state.cpsr.f.c = (op1 >= op2) && (result1 >= op3);
        state.cpsr.f.v = (((op1 ^ op2) & ~(result1 ^ op2)) ^ (result1 & ~result2)) >> 31;

        return result2;
    } else {
        return op1 - op2 - op3;
    }
}
