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

#include "gba_timer.h"

namespace NanoboyAdvance
{
    const int GBATimer::timings[4] = { 1, 64, 256, 1024 };

    GBATimer::GBATimer(GBAIO* gba_io)
    {
        // Assign our IO interface to the object
        this->gba_io = gba_io;
    }

    void GBATimer::Step()
    {
        bool timer0_enabled = (gba_io->tm0cnt_h & (1 << 7)) == (1 << 7);
        bool timer1_enabled = (gba_io->tm1cnt_h & (1 << 7)) == (1 << 7);
        bool timer2_enabled = (gba_io->tm2cnt_h & (1 << 7)) == (1 << 7);
        bool timer3_enabled = (gba_io->tm3cnt_h & (1 << 7)) == (1 << 7);
        bool timer1_countup = (gba_io->tm1cnt_h & (1 << 2)) == (1 << 2);
        bool timer2_countup = (gba_io->tm2cnt_h & (1 << 2)) == (1 << 2);
        bool timer3_countup = (gba_io->tm3cnt_h & (1 << 2)) == (1 << 2);
        int timer0_clock = timings[gba_io->tm0cnt_h & 3];
        int timer1_clock = timings[gba_io->tm1cnt_h & 3];
        int timer2_clock = timings[gba_io->tm2cnt_h & 3];
        int timer3_clock = timings[gba_io->tm3cnt_h & 3];
        bool timer0_overflow = false;
        bool timer1_overflow = false;
        bool timer2_overflow = false;

        // Handle Timer 0
        if (timer0_enabled && ++timer0_ticks >= timer0_clock)
        {
            timer0_ticks = 0;
            if (gba_io->tm0cnt_l != 0xFFFF)
            {
                gba_io->tm0cnt_l++;
            }
            else
            {
                if (gba_io->tm0cnt_h & (1 << 6))
                {
                    gba_io->if_ |= (1 << 3);
                }
                gba_io->tm0cnt_l = timer0_reload;
                timer0_overflow = true;
            }
        }

        // Handle Timer 1
        if (timer1_enabled && ((timer1_countup && timer0_overflow) || (!timer1_countup && ++timer1_ticks >= timer1_clock)))
        {
            timer1_ticks = 0;
            if (gba_io->tm1cnt_l != 0xFFFF)
            {
                gba_io->tm1cnt_l++;
            }
            else
            {
                if (gba_io->tm1cnt_h & (1 << 6))
                {
                    gba_io->if_ |= (1 << 4);
                }
                gba_io->tm1cnt_l = timer1_reload;
                timer1_overflow = true;
            }
        }

        // Handle Timer 2
        if (timer2_enabled && ((timer2_countup && timer1_overflow) || (!timer2_countup && ++timer2_ticks >= timer2_clock)))
        {
            timer2_ticks = 0;
            if (gba_io->tm2cnt_l != 0xFFFF)
            {
                gba_io->tm2cnt_l++;
            }
            else
            {
                if (gba_io->tm2cnt_h & (1 << 6))
                {
                    gba_io->if_ |= (1 << 5);
                }
                gba_io->tm2cnt_l = timer2_reload;
                timer2_overflow = true;
            }
        }

        // Handle Timer 3
        if (timer3_enabled && ((timer3_countup && timer2_overflow) || (!timer3_countup && ++timer3_ticks >= timer3_clock)))
        {
            timer3_ticks = 0;
            if (gba_io->tm3cnt_l != 0xFFFF)
            {
                gba_io->tm3cnt_l++;
            }
            else
            {
                if (gba_io->tm3cnt_h & (1 << 6))
                {
                    gba_io->if_ |= (1 << 6);
                }
                gba_io->tm3cnt_l = timer3_reload;
            }
        }
    }
}
