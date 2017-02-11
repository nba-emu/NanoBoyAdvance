///////////////////////////////////////////////////////////////////////////////////
//
//  ARMigo is an ARM7TDMI-S emulator/interpreter.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of ARMigo.
//
//  ARMigo is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  ARMigo is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ARMigo. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "cpu.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    constexpr int CPU::m_timer_ticks[4];

    void CPU::timer_step(int cycles)
    {
        bool overflow = false;

        for (int i = 0; i < 4; ++i)
        {
            auto& timer = m_io.timer[i];

            if (timer.control.enable)
            {
                if (timer.control.cascade && overflow)
                {
                    timer_increment(timer, overflow);
                }
                else if (!timer.control.cascade)
                {
                    timer.ticks += cycles;

                    // optimize. m_timer_ticks creates an actual lookup.
                    if (timer.ticks >= m_timer_ticks[timer.control.frequency])
                    {
                        timer_increment(timer, overflow);
                    }
                    else
                    {
                        overflow = false;
                    }
                }
                else
                {
                    overflow = false;
                }
            }
            else
            {
                overflow = false;
            }
        }
    }

    void CPU::timer_increment(struct io::timer& timer, bool& overflow)
    {
        timer.ticks = 0;

        if (timer.counter == 0xFFFF)
        {
            if (timer.control.interrupt)
            {
                m_interrupt.request((interrupt_type)(INTERRUPT_TIMER_0 << timer.id));
            }
            overflow = true;
            timer.counter = timer.reload;
        }
        else
        {
            overflow = false;
            timer.counter++;
        }
    }
}
