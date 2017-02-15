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

namespace Core {
    struct EmulatorConfig {
    };
    
    class Emulator {
    private:
        std::string m_game;
        std::string m_save;
        EmulatorConfig m_config;
        
    public:
        Emulator(EmulatorConfig config) {
            m_config = config;
        }
        
        void set_game_path(std::string path) {
            m_game = path;
        }
        
        void set_save_path(std::string path) {
            m_save = path;
        }
        
        virtual void reset();
        virtual void frame();
    };    
}