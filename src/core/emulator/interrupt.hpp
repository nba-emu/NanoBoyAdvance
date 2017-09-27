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

// TODO: get rid of this crap

namespace Core {
    class Interrupt {
    private:
        u16* m_interrupt_flag;

    public:
        void set_flag_register(u16* io_reg) {
            m_interrupt_flag = io_reg;
        }

        void request(InterruptType type) {
            *m_interrupt_flag |= static_cast<int>(type);
        }
    };
}
