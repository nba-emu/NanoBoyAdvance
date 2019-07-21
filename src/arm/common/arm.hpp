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

#pragma once

#include <cstdint>

namespace ARM {

enum Mode {
  MODE_USR = 0x10,
  MODE_FIQ = 0x11,
  MODE_IRQ = 0x12,
  MODE_SVC = 0x13,
  MODE_ABT = 0x17,
  MODE_UND = 0x1B,
  MODE_SYS = 0x1F
};

enum Bank {
  BANK_NONE = 0,
  BANK_FIQ  = 1,
  BANK_SVC  = 2,
  BANK_ABT  = 3,
  BANK_IRQ  = 4,
  BANK_UND  = 5,
  BANK_COUNT
};

enum Condition {
  COND_EQ = 0,
  COND_NE = 1,
  COND_CS = 2,
  COND_CC = 3,
  COND_MI = 4,
  COND_PL = 5,
  COND_VS = 6,
  COND_VC = 7,
  COND_HI = 8,
  COND_LS = 9,
  COND_GE = 10,
  COND_LT = 11,
  COND_GT = 12,
  COND_LE = 13,
  COND_AL = 14,
  COND_NV = 15
};

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

struct RegisterFile {
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
  
  RegisterFile() { Reset(); }
  
  void Reset() {
    for (int i = 0; i < 16; i++) reg[i] = 0;

    for (int i = 0; i < BANK_COUNT; i++) {
      for (int j = 0; j < 7; j++)
        bank[i][j] = 0;
      spsr[i].v = 0;
    }

    cpsr.v = 0;
  }
};

}
