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

#include "gba_dma.h"
#include "common/log.h"

namespace NanoboyAdvance
{
    GBADMA::GBADMA(Memory* memory, GBAIO* gba_io)
    {
        this->memory = memory;
        this->gba_io = gba_io;
    }

    void GBADMA::DMA0()
    {
        // Check enable bit
        if (gba_io->dma0cnt_h & (1 << 15))
        {
            AddressControl dest_control = static_cast<AddressControl>((gba_io->dma0cnt_h >> 5) & 3);
            AddressControl source_control = static_cast<AddressControl>((gba_io->dma0cnt_h >> 7) & 3);
            bool transfer_words = (gba_io->dma0cnt_h & (1 << 10)) == (1 << 10);
            while (dma0_count != 0)
            {
                // Perform transfer
                if (transfer_words)
                {
                    memory->WriteWord(dma0_destination, memory->ReadWord(dma0_source));
                }
                else
                {
                    memory->WriteHWord(dma0_destination, memory->ReadHWord(dma0_source));
                }
                // Update dest address
                switch (dest_control) { case Increment: case IncrementAndReload: dma0_destination += transfer_words ? 4 : 2; break; case Decrement: dma0_destination -= transfer_words ? 4 : 2; break; }
                // Update source address
                switch (source_control) { case Increment: case IncrementAndReload: dma0_source += transfer_words ? 4 : 2; break; case Decrement: dma0_source -= transfer_words ? 4 : 2; break; }
                // Update count
                dma0_count--;
            }
            // Check reload bit
            if (gba_io->dma0cnt_h & (1 << 9))
            {
                // Reload transfer count
                dma0_count = gba_io->dma0cnt_l;
                // Also reload source and dest address when destination mode is "IncrementAndReload"
                if (dest_control == AddressControl::IncrementAndReload)
                {
                    dma0_destination = gba_io->dma0dad;
                    dma0_source = gba_io->dma0sad;
                }
            }
            else
            {
                // Disable enable bit
                gba_io->dma0cnt_h &= ~(1 << 15);
            }
            // Check irq bit
            if (gba_io->dma0cnt_h & (1 < 14))
            {
                gba_io->if_ |= (1 << 8);
            }
        }
    }

    // TODO: Sepcial mode
    void GBADMA::DMA1()
    {
        // Check enable bit
        if (gba_io->dma1cnt_h & (1 << 15))
        {
            AddressControl dest_control = static_cast<AddressControl>((gba_io->dma1cnt_h >> 5) & 3);
            AddressControl source_control = static_cast<AddressControl>((gba_io->dma1cnt_h >> 7) & 3);
            bool transfer_words = (gba_io->dma1cnt_h & (1 << 10)) == (1 << 10);
            while (dma1_count != 0)
            {
                // Perform transfer
                if (transfer_words)
                {
                    memory->WriteWord(dma1_destination, memory->ReadWord(dma1_source));
                }
                else
                {
                    memory->WriteHWord(dma1_destination, memory->ReadHWord(dma1_source));
                }
                // Update dest address
                switch (dest_control) { case Increment: case IncrementAndReload: dma1_destination += transfer_words ? 4 : 2; break; case Decrement: dma1_destination -= transfer_words ? 4 : 2; break; }
                // Update source address
                switch (source_control) { case Increment: case IncrementAndReload: dma1_source += transfer_words ? 4 : 2; break; case Decrement: dma1_source -= transfer_words ? 4 : 2; break; }
                // Update count
                dma1_count--;
            }
            // Check reload bit
            if (gba_io->dma1cnt_h & (1 << 9))
            {
                // Reload transfer count
                dma1_count = gba_io->dma1cnt_l;
                // Also reload source and dest address when destination mode is "IncrementAndReload"
                if (dest_control == AddressControl::IncrementAndReload)
                {
                    dma1_destination = gba_io->dma1dad;
                    dma1_source = gba_io->dma1sad;
                }
            }
            else
            {
                // Disable enable bit
                gba_io->dma1cnt_h &= ~(1 << 15);
            }
            // Check irq bit
            if (gba_io->dma1cnt_h & (1 < 14))
            {
                gba_io->if_ |= (1 << 9);
            }
        }
    }

    // TODO: Sepcial mode
    void GBADMA::DMA2()
    {
        // Check enable bit
        if (gba_io->dma2cnt_h & (1 << 15))
        {
            AddressControl dest_control = static_cast<AddressControl>((gba_io->dma2cnt_h >> 5) & 3);
            AddressControl source_control = static_cast<AddressControl>((gba_io->dma2cnt_h >> 7) & 3);
            bool transfer_words = (gba_io->dma2cnt_h & (1 << 10)) == (1 << 10);
            while (dma2_count != 0)
            {
                // Perform transfer
                if (transfer_words)
                {
                    memory->WriteWord(dma2_destination, memory->ReadWord(dma2_source));
                }
                else
                {
                    memory->WriteHWord(dma2_destination, memory->ReadHWord(dma2_source));
                }
                // Update dest address
                switch (dest_control) { case Increment: case IncrementAndReload: dma2_destination += transfer_words ? 4 : 2; break; case Decrement: dma2_destination -= transfer_words ? 4 : 2; break; }
                // Update source address
                switch (source_control) { case Increment: case IncrementAndReload: dma2_source += transfer_words ? 4 : 2; break; case Decrement: dma2_source -= transfer_words ? 4 : 2; break; }
                // Update count
                dma2_count--;
            }
            // Check reload bit
            if (gba_io->dma2cnt_h & (1 << 9))
            {
                // Reload transfer count
                dma2_count = gba_io->dma2cnt_l;
                // Also reload source and dest address when destination mode is "IncrementAndReload"
                if (dest_control == AddressControl::IncrementAndReload)
                {
                    dma2_destination = gba_io->dma2dad;
                    dma2_source = gba_io->dma2sad;
                }
            }
            else
            {
                // Disable enable bit
                gba_io->dma2cnt_h &= ~(1 << 15);
            }
            // Check irq bit
            if (gba_io->dma2cnt_h & (1 < 14))
            {
                gba_io->if_ |= (1 << 10);
            }
        }
    }

    void GBADMA::DMA3()
    {
        // Check enable bit
        if (gba_io->dma3cnt_h & (1 << 15))
        {
            AddressControl dest_control = static_cast<AddressControl>((gba_io->dma3cnt_h >> 5) & 3);
            AddressControl source_control = static_cast<AddressControl>((gba_io->dma3cnt_h >> 7) & 3);
            bool transfer_words = (gba_io->dma3cnt_h & (1 << 10)) == (1 << 10);
            while (dma3_count != 0)
            {
                // Perform transfer
                if (transfer_words)
                {
                    memory->WriteWord(dma3_destination, memory->ReadWord(dma3_source));
                }
                else
                {
                    memory->WriteHWord(dma3_destination, memory->ReadHWord(dma3_source));
                }
                // Update dest address
                switch (dest_control) { case Increment: case IncrementAndReload: dma3_destination += transfer_words ? 4 : 2; break; case Decrement: dma3_destination -= transfer_words ? 4 : 2; break; }
                // Update source address
                switch (source_control) { case Increment: case IncrementAndReload: dma3_source += transfer_words ? 4 : 2; break; case Decrement: dma3_source -= transfer_words ? 4 : 2; break; }
                // Update count
                dma3_count--;
            }
            // Check reload bit
            if (gba_io->dma3cnt_h & (1 << 9))
            {
                // Reload transfer count
                dma3_count = gba_io->dma3cnt_l;
                // Also reload source and dest address when destination mode is "IncrementAndReload"
                if (dest_control == AddressControl::IncrementAndReload)
                {
                    dma3_destination = gba_io->dma3dad;
                    dma3_source = gba_io->dma3sad;
                }
            }
            else
            {
                // Disable enable bit
                gba_io->dma3cnt_h &= ~(1 << 15);
            }
            // Check irq bit
            if (gba_io->dma3cnt_h & (1 < 14))
            {
                gba_io->if_ |= (1 << 11);
            }
        }
    }

    void GBADMA::Step()
    {
        // TODO: Only check if enable bit is set?
        //       Do only trigger once per HBlank/Scanline?
        bool vblank = gba_io->dispstat & 1 ? true : false;
        bool hblank = gba_io->dispstat & 2 ? true : false;

        // DMA0
        switch ((gba_io->dma0cnt_h >> 12) & 3)
        {
        case 0:
            // Immediatly
            DMA0();
            break;
        case 1:
            // VBlank
            if (vblank)
            {
                DMA0();
            }
            break;
        case 2:
            // HBlank
            if (hblank)
            {
                DMA0();
            }
            break;
        case 3:
            LOG(LOG_ERROR, "Special not allowed for DMA0");
            break;
        }

        // DMA1
        switch ((gba_io->dma1cnt_h >> 12) & 3)
        {
        case 0:
            // Immediatly
            DMA1();
            break;
        case 1:
            // VBlank
            if (vblank)
            {
                DMA1();
            }
            break;
        case 2:
            // HBlank
            if (hblank)
            {
                DMA1();
            }
            break;
        case 3:
            //LOG(LOG_ERROR, "Unimplemented special for DMA1");
            break;
        }

        // DMA2
        switch ((gba_io->dma2cnt_h >> 12) & 3)
        {
        case 0:
            // Immediatly
            DMA2();
            break;
        case 1:
            // VBlank
            if (vblank)
            {
                DMA2();
            }
            break;
        case 2:
            // HBlank
            if (hblank)
            {
                DMA2();
            }
            break;
        case 3:
            //LOG(LOG_ERROR, "Unimplemented special for DMA2");
            break;
        }

        // DMA3
        switch ((gba_io->dma3cnt_h >> 12) & 3)
        {
        case 0:
            // Immediatly
            DMA3();
            break;
        case 1:
            // VBlank
            if (vblank)
            {
                DMA3();
            }
            break;
        case 2:
            // HBlank
            if (hblank)
            {
                DMA3();
            }
            break;
        case 3:
            if (hblank && gba_io->vcount >= 2 && gba_io->vcount <= 162)
            {
                DMA3();
            }
            break;
        }
    }
}