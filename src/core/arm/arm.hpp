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
        void signal_interrupt();
        
        auto get_context() -> ARMContext& {
            return ctx;
        }
        
        void set_context(ARMContext& ctx) {
            this->ctx = ctx;
        }
        
        bool get_fake_swi() const { 
            return fake_swi; 
        }
        
        void set_fake_swi(bool fake_swi) { 
            this->fake_swi = fake_swi; 
        }

    protected:

        // memory bus methods
        virtual u8  bus_read_byte(u32 address)  { return 0; }
        virtual u16 bus_read_hword(u32 address) { return 0; }
        virtual u32 bus_read_word(u32 address)  { return 0; }
        virtual void bus_write_byte(u32 address, u8 value) {}
        virtual void bus_write_hword(u32 address, u16 value) {}
        virtual void bus_write_word (u32 address, u32 value) {}

        // swi #nn HLE-handler
        virtual void software_interrupt(int number) {}

        // memory access helpers
        #include "memory.hpp"

    private:

        bool fake_swi;
        
        ARMContext ctx;
        
        void switch_mode(Mode new_mode);

        bool check_condition(Condition condition);
        void update_sign_flag(u32 result);
        void update_zero_flag(u64 result);
        void update_carry_flag(bool carry);
        void update_overflow_add(u32 result, u32 operand1, u32 operand2);
        void update_overflow_sub(u32 result, u32 operand1, u32 operand2);

        static void shift_lsl(u32& operand, u32 amount, bool& carry);
        static void shift_lsr(u32& operand, u32 amount, bool& carry, bool immediate);
        static void shift_asr(u32& operand, u32 amount, bool& carry, bool immediate);
        static void shift_ror(u32& operand, u32 amount, bool& carry, bool immediate);
        static void apply_shift(int shift, u32& operand, u32 amount, bool& carry, bool immediate);
        
        static Bank mode_to_bank(Mode mode);

        // ARM and THUMB interpreter cores
        #include "instr_arm.hpp"
        #include "instr_thumb.hpp"
    };
    
    #include "inline_code.hpp"
}