/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "arm/arm.hpp"
#include "arm/interface.hpp"
#include "config.hpp"
#include "event_device.hpp"
#include "ppu/ppu.hpp"

#include <unordered_set>

namespace NanoboyAdvance::GBA {

class CPU : private ARM::Interface {
public:
    CPU(Config* config);

    void Reset();
    void SetSlot1(uint8_t* rom, size_t size);
    void RegisterEvent(EventDevice& event);
    void UnregisterEvent(EventDevice& event);
    
    void RunFor(int cycles);
    
    enum class HaltControl {
        RUN,
        STOP,
        HALT
    };

    enum Interrupt {
        INT_VBLANK  = 1 << 0,
        INT_HBLANK  = 1 << 1,
        INT_VCOUNT  = 1 << 2,
        INT_TIMER0  = 1 << 3,
        INT_TIMER1  = 1 << 4,
        INT_TIMER2  = 1 << 5,
        INT_TIMER3  = 1 << 6,
        INT_SERIAL  = 1 << 7,
        INT_DMA0    = 1 << 8,
        INT_DMA1    = 1 << 9,
        INT_DMA2    = 1 << 10,
        INT_DMA3    = 1 << 11,
        INT_KEYPAD  = 1 << 12,
        INT_GAMEPAK = 1 << 13
    };

    enum DMAControl {
        DMA_INCREMENT = 0,
        DMA_DECREMENT = 1,
        DMA_FIXED     = 2,
        DMA_RELOAD    = 3
    };

    enum DMATime {
        DMA_IMMEDIATE = 0,
        DMA_VBLANK    = 1,
        DMA_HBLANK    = 2,
        DMA_SPECIAL   = 3
    };

    enum DMASize {
        DMA_HWORD = 0,
        DMA_WORD  = 1
    };
    
    Config* config;

    struct SystemMemory {
        std::uint8_t bios[0x04000];
        std::uint8_t wram[0x40000];
        std::uint8_t iram[0x08000];
        std::uint8_t pram[0x00400];
        std::uint8_t oam [0x00400];
        std::uint8_t vram[0x18000];

        struct ROM {
            std::uint8_t* data;
            size_t size;
        } rom;

        /* Last opcode fetched from BIOS memory. */
        std::uint32_t bios_opcode;
    } memory;

    struct MMIO {
        std::uint16_t keyinput;
        
        std::uint16_t irq_ie;
        std::uint16_t irq_if;
        std::uint16_t irq_ime;

        HaltControl haltcnt;

        struct WaitstateControl {
            int sram;
            int ws0_n;
            int ws0_s;
            int ws1_n;
            int ws1_s;
            int ws2_n;
            int ws2_s;
            int phi;
            int prefetch;
            int cgb;
        } waitcnt;
        
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

            int  ticks; /* cycles per increment */
            bool overflow;
        } timer[4];
        
        struct DMA {
            bool enable;
            bool repeat;
            bool interrupt;
            bool gamepak;

            std::uint16_t length;
            std::uint32_t dst_addr;
            std::uint32_t src_addr;
            DMAControl dst_cntl;
            DMAControl src_cntl;
            DMATime time;
            DMASize size;

            struct Internal {
                std::uint32_t length;
                std::uint32_t dst_addr;
                std::uint32_t src_addr;
            } internal;
        } dma[4];
    } mmio;

private:
    /* Contains memory read/write implementation. */
    #include "cpu.inl"

    void Tick(int cycles) final {
        run_until -= cycles;
    }

    void SWI(std::uint32_t call_id) final { }

    void ResetTimers();
    auto ReadTimer(int id, int offset) -> std::uint8_t;
    void WriteTimer(int id, int offset, std::uint8_t value);
    void RunTimers(int cycles);
    
    auto ReadMMIO(std::uint32_t address) -> std::uint8_t;
    void WriteMMIO(std::uint32_t address, std::uint8_t value);

    void UpdateCycleLUT();

    ARM::ARM7 cpu;
    PPU ppu;

    int run_until = 0;
    int go_for = 0;
    int cycles16[2][16];
    int cycles32[2][16];

    std::unordered_set<EventDevice*> events { &ppu };
    
    static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; /* Non-sequential SRAM/WS0/WS1/WS2 */
    static constexpr int s_ws_seq0[2] = { 2, 1 };       /* Sequential WS0 */
    static constexpr int s_ws_seq1[2] = { 4, 1 };       /* Sequential WS1 */
    static constexpr int s_ws_seq2[2] = { 8, 1 };       /* Sequential WS2 */
};

}
