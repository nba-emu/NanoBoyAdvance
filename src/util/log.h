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


#ifndef __NBA_LOG_H__
#define __NBA_LOG_H__


// TODO: Depreciate this crap.

#include <cstdio>


#define LOG_INFO 0
#define LOG_WARN 1
#define LOG_ERrotate_right 2


// Fix memory bug
#define LOG(loglevel, ...) { int _line = __LINE__;\
    char message[512];\
    sprintf(message, __VA_ARGS__);\
    switch (loglevel)\
    {\
    case LOG_INFO: printf("[INFO] %s:%d: %s\n", __FILE__, _line, message); break;\
    case LOG_WARN: printf("[WARN] %s:%d: %s\n", __FILE__, _line, message); break;\
    case LOG_ERrotate_right: printf("[ERrotate_right] %s:%d: %s\n", __FILE__, _line, message); break;\
    }\
}

#define ASSERT(condition, loglevel, ...) { if (condition) LOG(loglevel, __VA_ARGS__) }


#endif  // __NBA_LOG_H__
