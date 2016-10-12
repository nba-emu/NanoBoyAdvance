///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#ifndef __NBA_TIMER_HPP__
#define __NBA_TIMER_HPP__

#include "common/log.h"
namespace GBA
{
    // TODO: there seems to be a bug with the timer demo.
    class Timer
    {
    private:
        static constexpr int TMR_CYCLES[4] = { 1, 64, 256, 1024 };

    public:
        void Reset()
        {
            m_count  = 0;
            m_reload = 0;
            m_clock  = 0;
            m_ticks  = 0;
            m_enable    = false;
            m_countup   = false;
            m_interrupt = false;
            m_overflow  = false;
        }

        bool GetOverflow() { return m_overflow; }
        void AssignID(int id) { m_id = id; }

        u8 ReadCounterLow() { return m_count & 0xFF; }
        u8 ReadCounterHigh() { return m_count >> 8; }
        void WriteReloadLow(u8 value) { m_reload = (m_reload & 0xFF00) | value; }
        void WriteReloadHigh(u8 value) { m_reload = (m_reload & 0x00FF) | (value << 8); }

        u8 ReadControlRegister()
        {
            return m_clock | (m_countup ? 4 : 0) | (m_interrupt ? 64 : 0) | (m_enable ? 128 : 0);
        }

        void WriteControlRegister(u8 value)
        {
            bool old_enable = m_enable;

            m_clock   = value & 3;
            m_countup = value & 4;
            m_interrupt = value & 64;
            m_enable    = value & 128;

            // load reload value into counter on rising edge
            if (!old_enable && m_enable) m_count = m_reload;
        }

        void Step(bool& overflow)
        {
            if (m_enable)
            {
                if ((m_countup && overflow) || (!m_countup && ++m_ticks == TMR_CYCLES[m_clock]))
                {
                    Increment(overflow);
                }
                else
                {
                    overflow = false;
                }
            }

            overflow = false;
        }

    private:
        void Increment(bool& overflow);

        int m_id { 0 };
        u16 m_count  { 0 };
        u16 m_reload { 0 };
        int m_clock  { 0 };
        int m_ticks  { 0 };
        bool m_enable    { false };
        bool m_countup   { false };
        bool m_interrupt { false };
        bool m_overflow  { false };
    };
}


#endif // __NBA_TIMER_HPP__
