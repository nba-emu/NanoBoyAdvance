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

namespace Core {

    enum Layer {
        LAYER_BG0 = 0,
        LAYER_BG1 = 1,
        LAYER_BG2 = 2,
        LAYER_BG3 = 3,
        LAYER_OBJ = 4,
        LAYER_BD  = 5,
        LAYER_SFX = 5
    };
    
    enum SpecialEffect {
        SFX_NONE     = 0,
        SFX_BLEND    = 1,
        SFX_INCREASE = 2,
        SFX_DECREASE = 3
    };
    
    enum ObjectMode {
        OBJ_NORMAL     = 0,
        OBJ_SEMI       = 1,
        OBJ_WINDOW     = 2,
        OBJ_PROHIBITED = 3
    };
}
