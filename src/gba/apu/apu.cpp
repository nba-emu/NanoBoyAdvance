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

#include "apu.hpp"

namespace GameBoyAdvance {
    
    APU::APU() {
        // forward FIFO access to SOUNDCNT register (for FIFO resetting)
        m_io.control.fifo[0] = &m_io.fifo[0];
        m_io.control.fifo[1] = &m_io.fifo[1];
        
        // reset state
        reset();
    }
    
    void APU::reset() {
        m_io.fifo[0].reset();
        m_io.fifo[1].reset();
        m_io.bias.reset();
        m_io.control.reset();
    }
}