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

#include "context.hpp"

namespace GameBoyAdvance {

    class ARM {
    public:
        ARM();

        virtual void reset();

        void step();
        void signalIRQ();

        // ARM context getter/setter
        auto context() -> ARMContext& {
            return ctx;
        }
        void context(ARMContext& ctx) {
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

        ARMContext ctx;

        void switchMode(Mode new_mode);

        // Flag Helpers
        bool checkCondition(Condition condition);
        void updateSignFlag(u32 result);
        void updateZeroFlag(u64 result);
        void updateCarryFlag(bool carry);
        void updateOverflowFlagAdd(u32 result, u32 operand1, u32 operand2);
        void updateOverflowFlagSub(u32 result, u32 operand1, u32 operand2);

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

    #include "inline_code.hpp"
}
