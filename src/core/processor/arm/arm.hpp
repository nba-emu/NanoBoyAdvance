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

namespace Core {

    class ARM {
    private:
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

            // Processor Pipeline
            struct {
                int  index;
                u32  opcode[3];
                bool do_flush;
            } pipe;
        };

    public:
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
        virtual u8  busRead8 (u32 address, int flags) { return 0; }
        virtual u16 busRead16(u32 address, int flags) { return 0; }
        virtual u32 busRead32(u32 address, int flags) { return 0; }

        // System Write Methods
        virtual void busWrite8 (u32 address, u8 value,  int flags) {}
        virtual void busWrite16(u32 address, u16 value, int flags) {}
        virtual void busWrite32(u32 address, u32 value, int flags) {}

        // Internal Read Helpers
        u32 read8 (u32 address, int flags);
        u32 read16(u32 address, int flags);
        u32 read32(u32 address, int flags);

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
}
