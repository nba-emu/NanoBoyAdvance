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

#include "util/integer.hpp"

namespace Core {

    class ARM {
    public:
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
            BANK_NONE,
            BANK_FIQ,
            BANK_SVC,
            BANK_ABT,
            BANK_IRQ,
            BANK_UND,
            BANK_COUNT
        };

        enum BankedRegister {
            BANK_R13 = 0,
            BANK_R14 = 1
        };

        enum SavedStatusRegister {
            SPSR_DEF = 0,
            SPSR_FIQ = 1,
            SPSR_SVC = 2,
            SPSR_ABT = 3,
            SPSR_IRQ = 4,
            SPSR_UND = 5,
            SPSR_COUNT = 6
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

        enum StatusMask {
            MASK_MODE  = 0x1F,
            MASK_THUMB = 0x20,
            MASK_FIQD  = 0x40,
            MASK_IRQD  = 0x80,

            POS_VFLAG = 28,
            POS_CFLAG = 29,
            POS_ZFLAG = 30,
            POS_NFLAG = 31,

            MASK_VFLAG = 1 << POS_VFLAG /* 0x10000000 */,
            MASK_CFLAG = 1 << POS_CFLAG /* 0x20000000 */,
            MASK_ZFLAG = 1 << POS_ZFLAG /* 0x40000000 */,
            MASK_NFLAG = 1 << POS_NFLAG /* 0x80000000 */
        };

        enum ExceptionVector {
            EXCPT_RESET     = 0x00,
            EXCPT_UNDEFINED = 0x04,
            EXCPT_SWI       = 0x08,
            EXCPT_PREFETCH_ABORT = 0x0C,
            EXCPT_DATA_ABORT     = 0x10,
            EXCPT_INTERRUPT      = 0x18,
            EXCPT_FAST_INTERRUPT = 0x1C
        };

        enum MemoryFlags {
            M_NONE   = 0,

            // (Non-)Sequential Accesses
            M_SEQ    = (1 << 0),
            M_NONSEQ = (1 << 1),

            // Use for debug/internal accesses
            M_DEBUG  = (1 << 2),

            M_SIGNED = (1 << 3),
            M_ROTATE = (1 << 4),
            M_DMA    = (1 << 5)
        };

        struct Context {
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

            // Instruction Pipeline
            u32 pipe[2];
        };

        ARM();

        virtual void reset();

        void step();
        void signalIRQ();

        // ARM context getter/setter
        auto context() -> Context& {
            return ctx;
        }
        void context(Context& ctx) {
            this->ctx = ctx;
        }

        // SWI emulation flag getter/setter
        bool swiHLE() const {
            return fake_swi;
        }
        void swiHLE(bool fake_swi) {
            this->fake_swi = fake_swi;
        }

    protected:
        virtual void busInternalCycles(int count) {}

        // System Read Methods
        virtual auto busRead8 (u32 address, int flags) -> u8  { return 0; }
        virtual auto busRead16(u32 address, int flags) -> u16 { return 0; }
        virtual auto busRead32(u32 address, int flags) -> u32 { return 0; }

        // System Write Methods
        virtual void busWrite8 (u32 address, u8 value,  int flags) {}
        virtual void busWrite16(u32 address, u16 value, int flags) {}
        virtual void busWrite32(u32 address, u32 value, int flags) {}

        // Internal Read Helpers
        auto read8 (u32 address, int flags) -> u32;
        auto read16(u32 address, int flags) -> u32;
        auto read32(u32 address, int flags) -> u32;

        // Internal Write Helpers
        void write8 (u32 address, u8 value,  int flags);
        void write16(u32 address, u16 value, int flags);
        void write32(u32 address, u32 value, int flags);

        // Reloads Pipeline
        void refillPipeline();

        // swi #nn HLE-handler
        virtual void handleSWI(int number) {}

    private:

        bool fake_swi;

        Context ctx;

        void switchMode(Mode new_mode);

        // Flag Helpers
        bool checkCondition(Condition condition);
        void updateSignFlag(u32 result);
        void updateZeroFlag(u64 result);
        void updateCarryFlag(bool carry);
        void updateOverflowFlagAdd(u32 result, u32 operand1, u32 operand2);
        void updateOverflowFlagSub(u32 result, u32 operand1, u32 operand2);

        // Data Processing
        auto opDataProc(u32 result, bool set_nz, bool set_c = false, bool carry = false) -> u32;

        // ALU operations
        auto opADD(u32 op1, u32 op2, u32 op3,   bool set_flags) -> u32;
        auto opSUB(u32 op1, u32 op2,            bool set_flags) -> u32;
        auto opSBC(u32 op1, u32 op2, u32 carry, bool set_flags) -> u32;

        // Barrel Shifter Helpers
        static void shiftLSL(u32& operand, u32 amount, bool& carry);
        static void shiftLSR(u32& operand, u32 amount, bool& carry, bool immediate);
        static void shiftASR(u32& operand, u32 amount, bool& carry, bool immediate);
        static void shiftROR(u32& operand, u32 amount, bool& carry, bool immediate);
        static void applyShift(int shift, u32& operand, u32 amount, bool& carry, bool immediate);

        static Bank modeToBank(Mode mode);

        // ARM and THUMB interpreter cores
        #include "instr_arm.hpp"
        #include "instr_thumb.hpp"
    };

    #include "alu.hpp"
    #include "inline_code.hpp"
    #include "memory.hpp"
}
