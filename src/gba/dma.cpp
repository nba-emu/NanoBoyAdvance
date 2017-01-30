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

                //logger::log<LOG_DEBUG>("HBLANK DMA found: id={0}", dma.id);

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

                //logger::log<LOG_DEBUG>("VBLANK DMA found: id={0}", dma.id);

                return;
            }
        }
    }

    void cpu::dma_transfer_unit()
    {
        auto& dma = m_io.dma[m_current_dma];

        dma_control src_cntl = dma.src_cntl;
        dma_control dst_cntl = dma.dst_cntl;
        bool words = dma.size == DMA_WORD;

        while (dma.internal.length != 0)
        {
            if (words)
            {
                write_word(dma.internal.dst_addr, read_word(dma.internal.src_addr));
            }
            else
            {
                write_hword(dma.internal.dst_addr, read_hword(dma.internal.src_addr));
            }

            if (dma.dst_cntl == DMA_INCREMENT || dma.dst_cntl == DMA_RELOAD)
            {
                dma.internal.dst_addr += dma.size == DMA_WORD ? 4 : 2;
            }
            else if (dma.dst_cntl == DMA_DECREMENT)
            {
                dma.internal.dst_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            // TODO: DMA_RELOAD for src??
            if (dma.src_cntl == DMA_INCREMENT || dma.src_cntl == DMA_RELOAD)
            {
                dma.internal.src_addr += dma.size == DMA_WORD ? 4 : 2;
            }
            else if (dma.src_cntl == DMA_DECREMENT)
            {
                dma.internal.src_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            dma.internal.length--;
        }

        if (dma.repeat)
        {
            // TODO(accuracy): length, dst_addr must be masked.
            dma.internal.length = dma.length;

            if (dst_cntl == DMA_RELOAD)
            {
                dma.internal.dst_addr = dma.dst_addr;
            }
            //dma.internal.src_addr = dma.src_addr;

            // even though DMA will be repeated, we have to wait for it to be rescheduled.
            if (dma.time != DMA_IMMEDIATE)
            {
                m_dma_active = false;
                m_current_dma = -1;
            }
        }
        else
        {
            dma.enable = false;
            m_dma_active = false;
            m_current_dma = -1;
        }

        if (dma.interrupt)
        {
            m_interrupt.request((interrupt_type)(INTERRUPT_DMA_0 << dma.id));
        }

        /*if (dma.internal.length != 0)
        {
            dma.size == DMA_WORD ? write_word(dma.internal.dst_addr, read_word(dma.internal.src_addr)) :
                                   write_hword(dma.internal.dst_addr, read_hword(dma.internal.src_addr));

            if (dma.dst_cntl == DMA_INCREMENT || dma.dst_cntl == DMA_RELOAD)
            {
                dma.internal.dst_addr += dma.size == DMA_WORD ? 4 : 2;
            }
            else if (dma.dst_cntl == DMA_DECREMENT)
            {
                dma.internal.dst_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            // TODO: DMA_RELOAD for src??
            if (dma.src_cntl == DMA_INCREMENT || dma.src_cntl == DMA_RELOAD)
            {
                dma.internal.src_addr += dma.size == DMA_WORD ? 4 : 2;
            }
            else if (dma.src_cntl == DMA_DECREMENT)
            {
                dma.internal.src_addr -= dma.size == DMA_WORD ? 4 : 2;
            }

            dma.internal.length--;
            if (dma.internal.length == 0)
            {
                if (dma.repeat)
                {
                    // TODO(accuracy): length, dst/src_addr must be masked.
                    dma.internal.length = dma.length;
                    dma.internal.dst_addr = dma.dst_addr;
                    dma.internal.src_addr = dma.src_addr;

                    // even though DMA will be repeated, we have to wait for it to be rescheduled.
                    if (dma.time != DMA_IMMEDIATE)
                    {
                        m_dma_active = false;
                        m_current_dma = -1;
                    }
                }
                else
                {
                    dma.enable = false;
                    m_dma_active = false;
                    m_current_dma = -1;
                }

                if (dma.interrupt)
                {
                    m_interrupt.request((interrupt_type)(INTERRUPT_DMA_0 << dma.id));
                }
            }
        }*/
    }
}
