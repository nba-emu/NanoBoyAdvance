/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"

using namespace NanoboyAdvance::GBA;

static constexpr int g_ticks[4] = { 1, 64, 256, 1024 };

void CPU::ResetTimers() {
    for (int id = 0; id < 4; id++) {
        auto& timer = mmio.timer[id];

        timer.id = id;
        timer.cycles = 0;
        timer.reload = 0;
        timer.counter = 0;
        timer.control.frequency = 0;
        timer.control.cascade = false;
        timer.control.interrupt = false;
        timer.control.enable = false;
        timer.overflow = false;
    }
}

auto CPU::ReadTimer(int id, int offset) -> std::uint8_t {
    auto& timer = mmio.timer[id];

    switch (offset) {
        case 0: {
            return timer.counter & 0xFF;
        }
        case 1: {
            return timer.counter >> 8;
        }
        case 2: {
            return (timer.control.frequency) |
                   (timer.control.cascade   ? 4   : 0) |
                   (timer.control.interrupt ? 64  : 0) |
                   (timer.control.enable    ? 128 : 0);
        }
        default: return 0;
    }
}

void CPU::WriteTimer(int id, int offset, std::uint8_t value) {
    auto& timer = mmio.timer[id];

    switch (offset) {
        case 0: timer.reload = (timer.reload & 0xFF00) | (value << 0); break;
        case 1: timer.reload = (timer.reload & 0x00FF) | (value << 8); break;
        case 2: {
            bool enable_previous = timer.control.enable;

            timer.control.frequency = value & 3;
            timer.control.cascade   = value & 4;
            timer.control.interrupt = value & 64;
            timer.control.enable    = value & 128;

            timer.ticks = g_ticks[timer.control.frequency];

            if (!enable_previous && timer.control.enable) {
                timer.counter = timer.reload;
            }
        }
    }
}

void CPU::RunTimers(int cycles) {
    for (int id = 0; id < 4; id++) {
        auto& timer   = mmio.timer[id];
        auto& control = timer.control;

        if (!control.enable) {
            continue;
        }
        
        if (control.cascade) {
            if (id != 0 && mmio.timer[id - 1].overflow) {
                timer.overflow = false;
                if (timer.counter != 0xFFFF) {
                    timer.counter++;
                } else {
                    timer.counter  = timer.reload;
                    timer.overflow = true;
                    if (control.interrupt) {
                        mmio.irq_if |= CPU::INT_TIMER0 << id;
                    }
//                    if (timer.id < 2 && apu.getIO().control.master_enable) {
//                        timerHandleFIFO(id, 1);
//                    }
                }
                mmio.timer[id - 1].overflow = false;
            }
        } else {
            int available = timer.cycles + cycles;
            int overflows = 0;

            timer.overflow = false;
            
            while (available >= timer.ticks) {
                if (timer.counter != 0xFFFF) {
                    timer.counter++;
                } else {
                    timer.counter  = timer.reload;
                    timer.overflow = true;
                    overflows++;
                }
                available -= timer.ticks;
            }
            
            if (timer.overflow) {
                if (control.interrupt) {
                    mmio.irq_if |= CPU::INT_TIMER0 << id;
                }
//                if (timer.id < 2 && apu.getIO().control.master_enable) {
//                    timerHandleFIFO(id, overflows);
//                }
            }
            timer.cycles = available;
        }
    }
}