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

namespace NanoboyAdvance
{
    class ARM7Breakpoint
    {
    public:
        // Enum describing different types of breakpoints
        enum class ARM7BreakpointType
        {
            Execute,
            Read,
            Write,
            Access,
            StepOver,
            SVC
        };

        // Which type of breakpoint to employ
        ARM7BreakpointType breakpoint_type{ ARM7BreakpointType::Execute };

        // The breakpoint address (unused in case of "step over" breakpoint)
        uint concerned_address{ 0 };

        // The bios call on which to break (unused in any other breakpoint type than SVC)
        uint bios_call{ 0 };

        // Execute breakpoints only, indicates if the target opcode is a thumb or arm instruction
        bool thumb_mode{ false };
    };
}
