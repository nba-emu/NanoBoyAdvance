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
    void cpu::dma_hblank()
    {
        for (int i = 0; i < 4; i++)
        {
            auto& dma = m_io.dma[i];

            if (dma.enable && dma.time == DMA_HBLANK)
            {
                m_dma_active = true;
                m_current_dma = i;

                logger::log<LOG_DEBUG>("HBLANK DMA found: id={0}", dma.id);

                return;
            }
        }
    }

    void cpu::dma_vblank()
    {
        for (int i = 0; i < 4; i++)
        {
            auto& dma = m_io.dma[i];

            if (dma.enable && dma.time == DMA_VBLANK)
            {
                m_dma_active = true;
                m_current_dma = i;

                logger::log<LOG_DEBUG>("VBLANK DMA found: id={0}", dma.id);

                return;
            }
        }
    }

    void cpu::dma_transfer_unit()
    {
    }
}
