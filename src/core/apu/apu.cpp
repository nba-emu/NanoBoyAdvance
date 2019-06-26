/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "apu.hpp"
#include "../cpu.hpp"

#include <cstdio>

using namespace NanoboyAdvance::GBA;

void APU::Reset() {
    mmio.soundcnt.Reset();
    fifo[0].Reset();
    fifo[1].Reset();
}
    
void APU::LatchFIFO(int id, int times) {
    for (int time = 0; time < times; time++) {
        latch[id] = fifo[id].Read();
        //std::printf("latched %d\n", latch[id]);
        
        // HACK: we should match FIFO the DMA.
        if (fifo[id].Count() <= 16) {
            cpu->dma.RunFIFO(1 + id);
        }
    }
}