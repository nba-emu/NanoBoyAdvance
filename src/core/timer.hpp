/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance::GBA {

class CPU;
    
class TimerController {
public:
    TimerController(CPU* cpu) : cpu(cpu) { }
    
    void Reset();
    auto Read(int id, int offset) -> std::uint8_t;
    void Write(int id, int offset, std::uint8_t value);
    void Run(int cycles);
    
private:
    void RunFIFO(int timer_id, int times);
    
    CPU* cpu;

    struct Timer {
        int id;

        struct Control {
            int frequency;
            bool cascade;
            bool interrupt;
            bool enable;
        } control;

        int cycles;
        std::uint16_t reload;
        std::uint32_t counter;

        /* internal */
        int  shift;
        int  mask;
        bool overflow;
    } timer[4];
};

}