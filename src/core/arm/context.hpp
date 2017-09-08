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

#pragma once

#include "enums.hpp"
#include "util/integer.hpp"

namespace GameBoyAdvance {
    struct ARMContext {
        // General Purpose Registers
        union {
            struct {
                u32 r0;
                u32 r1;
                u32 r2;
                u32 r3;
                u32 r4;
                u32 r5;
                u32 r6;
                u32 r7;
                u32 r8;
                u32 r9;
                u32 r10;
                u32 r11;
                u32 r12;
                u32 r13;
                u32 r14;
                u32 r15;
            };
            u32 reg[16];
        };
        u32 bank[BANK_COUNT][7];

        // Program Status Registers
        u32  cpsr;
        u32  spsr[SPSR_COUNT];
        u32* p_spsr;

        // Processor Pipeline
        struct {
            int  index;
            u32  opcode[3];
            bool do_flush;
        } pipe;
    };
}
