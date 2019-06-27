/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "apu.hpp"
#include "../cpu.hpp"

using namespace NanoboyAdvance::GBA;

void APU::Reset() {
    mmio.fifo[0].Reset();
    mmio.fifo[1].Reset();
    mmio.soundcnt.Reset();
    
    dump = fopen("audio.raw", "wb");
    wait_cycles = 512;
}
    
void APU::LatchFIFO(int id, int times) {
    auto& fifo = mmio.fifo[id];
    
    for (int time = 0; time < times; time++) {
        latch[id] = fifo.Read();
        if (fifo.Count() <= 16) {
            cpu->dma.RequestFIFO(id);
        }
    }
}

bool APU::Tick() {
    fwrite(&latch, 2, 1, dump);
    wait_cycles += 512;
    return true;
}