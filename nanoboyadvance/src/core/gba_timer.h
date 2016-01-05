/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common/types.h"
#include "gba_io.h"

using namespace std;

namespace NanoboyAdvance
{
    class GBATimer
    {
        GBAIO* gba_io;
        static const int timings[4];
        bool timer_enabled[4];
        bool timer_countup[4];
        bool timer_interrupt[4];
        int timer_clock[4];
        int timer_ticks[4];
        void ScheduleTimer(int index, ushort& counter, bool& overflow);
    public:
        bool timer0_altered {false}, timer1_altered {false}, timer2_altered {false}, timer3_altered {false};
        ushort timer_reload[4];
        GBATimer(GBAIO* gba_io);
        void Step();
    };
}
