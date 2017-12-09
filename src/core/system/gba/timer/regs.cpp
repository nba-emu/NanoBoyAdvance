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

#include "../emulator.hpp"

namespace Core {
    static constexpr int g_ticks[4] = { 1, 64, 256, 1024 };

    void Emulator::timerReset(int id) {
        auto& timer = regs.timer[id];

        timer.id      = id;
        timer.cycles  = 0;
        timer.reload  = 0;
        timer.counter = 0;
        timer.control.frequency = 0;
        timer.control.cascade   = false;
        timer.control.interrupt = false;
        timer.control.enable    = false;

        timer.overflow = false;
    }

    auto Emulator::timerRead(int id, int offset) -> u8 {
        auto& timer = regs.timer[id];

        switch (offset) {
            case 0: {
                return timer.counter & 0xFF;
            }
            case 1: {
                return timer.counter >> 8;
            }
            case 2: {
                return (timer.control.frequency) |
                       (timer.control.cascade   ? 4   : 0) |
                       (timer.control.interrupt ? 64  : 0) |
                       (timer.control.enable    ? 128 : 0);
            }
            default: return 0;
        }
    }

    void Emulator::timerWrite(int id, int offset, u8 value) {
        auto& timer = regs.timer[id];

        switch (offset) {
            case 0: timer.reload = (timer.reload & 0xFF00) | (value << 0); break;
            case 1: timer.reload = (timer.reload & 0x00FF) | (value << 8); break;
            case 2: {
                bool enable_previous = timer.control.enable;

                timer.control.frequency = value & 3;
                timer.control.cascade   = value & 4;
                timer.control.interrupt = value & 64;
                timer.control.enable    = value & 128;

                timer.ticks = g_ticks[timer.control.frequency];

                if (!enable_previous && timer.control.enable) {
                    timer.counter = timer.reload;
                }
            }
        }
    }
}
