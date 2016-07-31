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


#ifndef __NBA_BACKUP_H__
#define __NBA_BACKUP_H__


#include "util/types.h"


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    backup.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   GBABackup
    /// \brief   Serves as a base class for GBA memory types.
    ///
    ///////////////////////////////////////////////////////////
    class GBABackup
    {
    public:

        virtual u8 ReadByte(u32 offset) { return 0; }
        virtual void WriteByte(u32 offset, u8 value) {}
        virtual ~GBABackup() {}
    };
}


#endif  // __NBA_BACKUP_H__
