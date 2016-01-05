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

#include <string.h>
#include "gba_timer.h"

namespace NanoboyAdvance
{
    const int GBATimer::timings[4] = { 1, 64, 256, 1024 };

    GBATimer::GBATimer(GBAIO* gba_io)
    {
        // Assign our IO interface to the object
        this->gba_io = gba_io;

        // Zero-init memory
        memset(timer_enabled, 0, 4 * sizeof(bool));
        memset(timer_countup, 0, 4 * sizeof(bool));
        memset(timer_interrupt, 0, 4 * sizeof(bool));
        memset(timer_clock, 0, 4 * sizeof(int));
        memset(timer_ticks, 0, 4 * sizeof(int));
        memset(timer_reload, 0, 4 * sizeof(ushort));

        // Timer0 has now countup...
        timer_countup[0] = false;
    }

    void GBATimer::ScheduleTimer(int index, ushort& counter, bool& overflow)
    {
        if ((timer_countup[index] && overflow) || (!timer_countup[index] && ++timer_ticks[index] >= timer_clock[index]))
        {
            timer_ticks[index] = 0;
            if (counter != 0xFFFF)
            {
                counter++;
            }
            else
            {
                if (timer_interrupt[index])
                {
                    gba_io->if_ |= (1 << (3 + index));
                }
                counter = timer_reload[index];
                overflow = true;
            }
        }
    }

    void GBATimer::Step()
    {
        bool overflow = false;

        // Detect if timer0 settings where altered. If so reload its settings.
        if (timer0_altered)
        {
            timer_enabled[0] = (gba_io->tm0cnt_h & (1 << 7)) == (1 << 7);
            timer_interrupt[0] = (gba_io->tm0cnt_h & (1 << 6)) == (1 << 6);
            timer_clock[0] = timings[gba_io->tm0cnt_h & 3];
            timer0_altered = false;
        }

        // Detect if timer1 settings where altered. If so reload its settings.
        if (timer1_altered)
        {
            timer_enabled[1] = (gba_io->tm1cnt_h & (1 << 7)) == (1 << 7);
            timer_countup[1] = (gba_io->tm1cnt_h & (1 << 2)) == (1 << 2);
            timer_interrupt[1] = (gba_io->tm1cnt_h & (1 << 6)) == (1 << 6);
            timer_clock[1] = timings[gba_io->tm1cnt_h & 3];
            timer1_altered = false;
        }

        // Detect if timer2 settings where altered. If so reload its settings.
        if (timer2_altered)
        {
            timer_enabled[2] = (gba_io->tm2cnt_h & (1 << 7)) == (1 << 7);
            timer_countup[2] = (gba_io->tm2cnt_h & (1 << 2)) == (1 << 2);
            timer_interrupt[2] = (gba_io->tm2cnt_h & (1 << 6)) == (1 << 6);
            timer_clock[2] = timings[gba_io->tm2cnt_h & 3];
            timer2_altered = false;
        }

        // Detect if timer3 settings where altered. If so reload its settings.
        if (timer3_altered)
        {
            timer_enabled[3] = (gba_io->tm3cnt_h & (1 << 7)) == (1 << 7);
            timer_countup[3] = (gba_io->tm3cnt_h & (1 << 2)) == (1 << 2);
            timer_interrupt[3] = (gba_io->tm3cnt_h & (1 << 6)) == (1 << 6);
            timer_clock[3] = timings[gba_io->tm3cnt_h & 3];
            timer3_altered = false;
        }

        // Run all the *enabled* timers
        if (timer_enabled[0]) ScheduleTimer(0, gba_io->tm0cnt_l, overflow);
        if (timer_enabled[1]) ScheduleTimer(1, gba_io->tm1cnt_l, overflow);
        if (timer_enabled[2]) ScheduleTimer(2, gba_io->tm2cnt_l, overflow);
        if (timer_enabled[3]) ScheduleTimer(3, gba_io->tm3cnt_l, overflow);
    }
}
