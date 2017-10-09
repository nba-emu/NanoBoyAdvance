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

#include "regs.hpp"

namespace Core {
    void Timer::reset() {
        cycles  = 0;
        reload  = 0;
        counter = 0;
        control.frequency = 0;
        control.cascade   = false;
        control.interrupt = false;
        control.enable    = false;
    }

    auto Timer::read(int offset) -> u8 {
        switch (offset) {
            case 0: return counter & 0xFF;
            case 1: return counter >> 8;
            case 2:
                return (control.frequency) |
                       (control.cascade   ? 4   : 0) |
                       (control.interrupt ? 64  : 0) |
                       (control.enable    ? 128 : 0);
        }
    }

    void Timer::write(int offset, u8 value) {
        switch (offset) {
            case 0: reload = (reload & 0xFF00) | (value << 0); break;
            case 1: reload = (reload & 0x00FF) | (value << 8); break;
            case 2: {
                bool enable_previous = control.enable;

                control.frequency = value & 3;
                control.cascade   = value & 4;
                control.interrupt = value & 64;
                control.enable    = value & 128;

                if (!enable_previous && control.enable) {
                    counter = reload;
                }
            }
        }
    }
}
