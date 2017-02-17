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

namespace GameBoyAdvance {
    const int FIFO_SIZE = 32;
    
    class FIFO {
    private:
        int m_index;
        s8  m_buffer[FIFO_SIZE];
        
    public:
        FIFO() {
            reset();
        }
        
        reset() {
            m_index = 0;
        }
        
        bool requires_data() {
            return m_index <= (FIFO_SIZE >> 1);
        }
        
        void enqueue(u8 data) {
            if (m_index < FIFO_SIZE) {
                m_buffer[m_index++] = static_cast<s8>(data);
            }
        }
        
        auto dequeue() -> s8 {
            s8 value;
            
            if (m_index == 0) {
                // FIFO underrun
                return 0;
            }
            
            value = m_buffer[0];
            
            // advances each next sample by one position
            for (int i = 1; i < m_index; i++) {
                m_buffer[i - 1] = m_buffer[i];
            }
            
            m_index--;
            
            return value;
        }
    };
    
}