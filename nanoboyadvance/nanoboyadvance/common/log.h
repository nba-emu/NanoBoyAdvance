/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <cstdio>

#define LOG_INFO 0
#define LOG_WARN 1
#define LOG_ERROR 2

#define LOG(loglevel, ...) { int line = __LINE__;\
    char message[512];\
    sprintf_s(message, 512, __VA_ARGS__);\
    switch (loglevel)\
    {\
    case LOG_INFO: printf("[INFO] %s:%d: %s\n", __FILE__, line, message); break;\
    case LOG_WARN: printf("[WARN] %s:%d: %s\n", __FILE__, line, message); break;\
    case LOG_ERROR: printf("[ERROR] %s:%d: %s\n", __FILE__, line, message); break;\
    }\
}

#define ASSERT(condition, loglevel, ...) { if (condition) LOG(loglevel, __VA_ARGS__) }