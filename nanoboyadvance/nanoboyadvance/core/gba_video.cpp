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

#include "gba_video.h"

namespace NanoboyAdvance
{
    GBAVideo::GBAVideo(GBAMemory* memory)
    {
        this->memory = memory;
        state = GBAVideoState::Scanline;
        ticks = 0;
    }

    void NanoboyAdvance::GBAVideo::Step()
    {
        ticks++;
        switch (state)
        {
        case GBAVideoState::Scanline:
            if (ticks >= 960)
            {
                ticks = 0;
                memory->gba_io->dispstat = (memory->gba_io->dispstat & ~3) | 2; // set hblank bit
                state = GBAVideoState::HBlank;
                // This is the point where the scanline should be rendered
            }
            break;
        case GBAVideoState::HBlank:
            if (ticks >= 272)
            {
                ticks = 0;
                memory->gba_io->dispstat = memory->gba_io->dispstat & ~2; // clear hblank bit
                memory->gba_io->vcount++;
                if (memory->gba_io->vcount == 160)
                {
                    memory->gba_io->dispstat = (memory->gba_io->dispstat & ~3) | 1; // set vblank bit
                    state = GBAVideoState::VBlank;
                }
                else
                {
                    state = GBAVideoState::Scanline;
                }
            }
            break;
        case GBAVideoState::VBlank:
            if (ticks >= 1232)
            {
                ticks = 0;
                memory->gba_io->vcount++;
                if (memory->gba_io->vcount >= 227) // check wether this must be 227 or 228
                {
                    state = GBAVideoState::Scanline;
                    memory->gba_io->dispstat = memory->gba_io->dispstat & ~3; // clear vblank and hblank bit
                    memory->gba_io->vcount = 0;
                }
            }
            break;
        }
    }
}
