/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include <cstring>
#include <stdexcept>
#include "sram.hpp"
#include "util/file.hpp"

using namespace Util;

namespace Core {
    
    SRAM::SRAM(std::string save_file) {
        m_save_file = save_file;
        reset();
    }
    
    SRAM::~SRAM() {
        File::write_data(m_save_file, m_memory, SRAM_SIZE);
    }
    
    void SRAM::reset() {
        if (File::exists(m_save_file)) {
            int size = File::get_size(m_save_file);
            
            if (size == SRAM_SIZE) {
                u8* data = File::read_data(m_save_file);
                
                std::memcpy(m_memory, data, SRAM_SIZE);
            } else {
                throw std::runtime_error("invalid SRAM save: " + m_save_file);
            }
        } else {
            std::memset(m_memory, 0, SRAM_SIZE);
        }
    }
    
    auto SRAM::read8(u32 address) -> u8 {
        return m_memory[address & 0x7FFF];
    }
    
    void SRAM::write8(u32 address, u8 value) {
        m_memory[address & 0x7FFF] = value;
    }
    
}