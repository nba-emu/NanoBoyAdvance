///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
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

#include "cpu.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    u8 cpu::read_io(u32 address)
    {
        logger::log<LOG_INFO>("IO read from 0x{0:x} r15={1:x}", address, m_reg[15]);

        switch (address)
        {
        // DISPSTAT
        case 0x04000004:
            return m_ppu.get_io().status.read(0);
        case 0x04000005:
            return m_ppu.get_io().status.read(1);

        // VCOUNT
        case 0x04000006:
            return m_ppu.get_io().vcount & 0xFF;
        case 0x04000007:
            return m_ppu.get_io().vcount >> 8;

        // KEYINPUT
        case 0x04000130:
            return m_io.keyinput & 0xFF;
        case 0x04000131:
            return m_io.keyinput >> 8;
        }

        return 0;
    }

    void cpu::write_io(u32 address, u8 value)
    {
        logger::log<LOG_INFO>("IO write to 0x{0:x}={1:x} r15={2:x}", address, value, m_reg[15]);

        switch (address)
        {
        // DISPSTAT
        case 0x04000004:
            return m_ppu.get_io().status.write(0, value);
        case 0x04000005:
            return m_ppu.get_io().status.write(1, value);
        }
    }
}
