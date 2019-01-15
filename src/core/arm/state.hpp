/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include "enums.hpp"

namespace ARM {

/* Bank Registers */
enum BankedRegister {
    BANK_R13 = 0,
    BANK_R14 = 1
};

typedef union {
    struct {
        enum Mode mode : 5;
        unsigned int thumb : 1;
        unsigned int mask_fiq : 1;
        unsigned int mask_irq : 1;
        unsigned int reserved : 19;
        unsigned int q : 1;
        unsigned int v : 1;
        unsigned int c : 1;
        unsigned int z : 1;
        unsigned int n : 1;
    } f;
    std::uint32_t v;
} StatusRegister;

/* ARM7TDMI-S execution state. */
struct State {
    // General Purpose Registers
    union {
        struct {
            std::uint32_t r0;
            std::uint32_t r1;
            std::uint32_t r2;
            std::uint32_t r3;
            std::uint32_t r4;
            std::uint32_t r5;
            std::uint32_t r6;
            std::uint32_t r7;
            std::uint32_t r8;
            std::uint32_t r9;
            std::uint32_t r10;
            std::uint32_t r11;
            std::uint32_t r12;
            std::uint32_t r13;
            std::uint32_t r14;
            std::uint32_t r15;
        };
        std::uint32_t reg[16];
    };
    
    // Banked Registers
    std::uint32_t bank[BANK_COUNT][7];

    // Program Status Registers
    StatusRegister cpsr;
    StatusRegister spsr[BANK_COUNT];
};

}
