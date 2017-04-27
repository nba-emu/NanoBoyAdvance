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
#define ARMIGO_INCLUDE

namespace GameBoyAdvance {

    class ARM {
    public:
        ARM();
        virtual void Reset();
        
        void Step();
        void RaiseInterrupt();
        // TODO: Fast Interrupt (FIQ)
        
        auto GetContext() -> ARMContext& {
            return ctx;
        }
        void SetContext(ARMContext& ctx) {
            this->ctx = ctx;
        }
        
        bool GetFakeSWI() const { 
            return fake_swi; 
        }
        void SetFakeSWI(bool fake_swi) { 
            this->fake_swi = fake_swi; 
        }

    protected:
        
        ARMContext ctx;
        
        bool fake_swi;

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
        static Bank mode_to_bank(Mode mode);

        void switch_mode(Mode new_mode);

        // conditional helpers
        #include "flags.hpp"

        // shifting helpers
        #include "shifting.hpp"

        // ARM and THUMB interpreter cores
        #include "arm/arm_emu.hpp"
        #include "thumb/thumb_emu.hpp"
    };
}

#undef ARMIGO_INCLUDE
