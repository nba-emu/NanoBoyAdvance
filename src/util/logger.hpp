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

#include <map>
#include <string>
#include "third_party/fmt/printf.h"

namespace Util {
    
    enum LogLevel {
        LOG_TRACE = 0,
        LOG_DEBUG = 1,
        LOG_INFO  = 2,
        LOG_WARN  = 3,
        LOG_ERROR = 4,
        LOG_FATAL = 5
    };

    namespace Logger {
        
        extern std::map<int, std::string> g_level_map;

        template <LogLevel level, typename... Parameters>
        void log(std::string message, Parameters... parameters) {
        
#ifdef DEBUG
            fmt::print("[" + g_level_map[level] + "] " + message + "\n", parameters...);
#endif
        }
    }
}
