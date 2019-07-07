/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "apu.hpp"
#include "../cpu.hpp"
#include <math.h>

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

#define POINTS (32)

bool APU::Tick() {
    float T = 32768.0/48000.0;
    static float t = 0.0;
    
    static float meme[2][POINTS] {0};
    
    for (int i = 1; i < POINTS; i++) {
        meme[0][i-1] = meme[0][i];
        meme[1][i-1] = meme[1][i];
    }
    meme[0][POINTS-1] = latch[0]/128.0;
    meme[1][POINTS-1] = latch[1]/128.0;

    while (t < 1.0) {
        int16_t foo[2] { 0 };
        
//        float a = (1.0 - cos(M_PI*t))*0.5;
//        float b = 1.0 - a;
//        
//        foo[0] = int16_t(round((meme[0][POINTS-2] * b + meme[0][POINTS-1] * a) * 32767.0));
//        foo[1] = int16_t(round((meme[1][POINTS-2] * b + meme[1][POINTS-1] * a) * 32767.0));

        float foo2[2] {0};
        
        for (int n = 0; n < POINTS; n++) {
            float x = t - (n - POINTS/2);
            float sinc = (x == 0) ? 1.0 : (sin(M_PI*x)/(M_PI*x));
            
            float a0 = 0.42;
            float a1 = 0.5;
            float a2 = 0.08;
            float win = a0 - a1*cos(2*M_PI*n/POINTS) + a2*cos(4*M_PI*n/POINTS);

            foo2[0] += sinc * win * meme[0][n];
            foo2[1] += sinc * win * meme[1][n];
        }
        
        foo[0] = int16_t(round(foo2[0] * 32767.0));
        foo[1] = int16_t(round(foo2[1] * 32767.0));
        
        fwrite(foo, 2, sizeof(int16_t), dump);
        t += T;
    }
    
    t = t - 1.0;
   
    wait_cycles += 512;
    return true;
}