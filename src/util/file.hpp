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

#pragma once

#include <string>
#include "integer.hpp"

namespace Util {
  
  namespace File {
    
    /// Determines wether a file exists.
    /// @param  filename  the file to check.
    /// @returns  wether the file exists.
    bool exists(std::string filename);

    /// Determines file size.
    /// @param  filename  the file to get the size from.
    /// @returns  the file size.
    int get_size(std::string filename);

    /// Reads a file into a byte array.
    /// @param  filename  the file to read.
    /// @returns  the byte array.
    u8* read_data(std::string filename);

    /// Writes a byte array to a file.
    /// @param  filename  the file to write
    /// @param  data    the byte array
    /// @param  size    the size of the array
    void write_data(std::string filename, u8* data, int size);
  }
}
