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

#pragma once

#include "util/integer.hpp"

namespace Core {
    class FIFO {
    private:
        static const int s_fifo_size = 32;

        int index;
        s8  buffer[s_fifo_size];

    public:
        FIFO() {
            reset();
        }

        void reset() {
            index = 0;
        }

        bool requiresData() {
            return index <= (s_fifo_size >> 1);
        }

        void enqueue(u8 data) {
            if (index < s_fifo_size) {
                buffer[index++] = static_cast<s8>(data);
            }
        }

        auto dequeue() -> s8 {
            s8 value;
            if (index == 0) {
                return 0;
            }
            value = buffer[0];
            for (int i = 1; i < index; i++) {
                buffer[i - 1] = buffer[i];
            }
            index--;
            return value;
        }
    };

}
