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

#include <string>
#include "util/integer.hpp"

namespace Core {

    struct Config {
        // Core
        std::string bios_path;

        // Get rid of these.
        int  frameskip     = 0;
        bool darken_screen = false;

        // Rendering buffer
        u32* framebuffer   = nullptr;

        struct Audio {
            int sample_rate;
            int buffer_size;
            
            struct AudioMute {
                bool psg[4] { false, false, false, false };
                bool dma[2] { false, false };

                bool master { false };
            } mute;
        } audio;

        // Misc. Get rid of these.
        int  multiplier   = 1;
        bool fast_forward = false;
    };
}
