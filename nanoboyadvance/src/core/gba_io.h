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

#define DISPCNT 0x0
#define DISPSTAT 0x4
#define VCOUNT 0x6
#define BG0CNT 0x8
#define BG1CNT 0xA
#define BG2CNT 0xC
#define BG3CNT 0xE
#define BG0HOFS 0x10
#define BG0VOFS 0x12
#define BG1HOFS 0x14
#define BG1VOFS 0x16
#define BG2HOFS 0x18
#define BG2VOFS 0x1A
#define BG3HOFS 0x1C
#define BG3VOFS 0x1E
#define BG2PA 0x20
#define BG2PB 0x22
#define BG2PC 0x24
#define BG2PD 0x26
#define BG2X 0x28
#define BG2Y 0x2C
#define BG3PA 0x30
#define BG3PB 0x32
#define BG3PC 0x34
#define BG3PD 0x36
#define BG3X 0x38
#define BG3Y 0x3C
#define WIN0H 0x40
#define WIN1H 0x42
#define WIN0V 0x44
#define WIN1V 0x46
#define WININ 0x48
#define WINOUT 0x4A
#define DMA0SAD 0xB0
#define DMA0DAD 0xB4
#define DMA0CNT_L 0xB8
#define DMA0CNT_H 0xBA
#define DMA1SAD 0xBC
#define DMA1DAD 0xC0
#define DMA1CNT_L 0xC4
#define DMA1CNT_H 0xC6
#define DMA2SAD 0xC8
#define DMA2DAD 0xCC
#define DMA2CNT_L 0xD0
#define DMA2CNT_H 0xD2
#define DMA3SAD 0xD4
#define DMA3DAD 0xD8
#define DMA3CNT_L 0xDC
#define DMA3CNT_H 0xDE
#define TM0CNT_L 0x100
#define TM0CNT_H 0x102
#define TM1CNT_L 0x104
#define TM1CNT_H 0x106
#define TM2CNT_L 0x108
#define TM2CNT_H 0x10A
#define TM3CNT_L 0x10C
#define TM3CNT_H 0x10E
#define IE 0x200
#define IF 0x202
#define IME 0x208
#define HALTCNT 0x301

// Fields with prefixed underscore shouldn't be accessed per GBAIO interface
// because they are implemented in a special manner (they are placeholders).
#pragma pack(push, r1, 1)
struct GBAIO_S
{
    // LCD I/O Registers
    u16 dispcnt;
    u16 greenswap;
    u16 dispstat;
    u16 vcount;
    u16 bg0cnt;
    u16 bg1cnt;
    u16 bg2cnt;
    u16 bg3cnt;
    u16 bg0hofs;
    u16 bg0vofs;
    u16 bg1hofs;
    u16 bg1vofs;
    u16 bg2hofs;
    u16 bg2vofs;
    u16 bg3hofs;
    u16 bg3vofs;
    u16 bg2pa;
    u16 bg2pb;
    u16 bg2pc;
    u16 bg2pd;
    u32 bg2x;
    u32 bg2y;
    u16 bg3pa;
    u16 bg3pb;
    u16 bg3pc;
    u16 bg3pd;
    u32 bg3x;
    u32 bg3y;
    u16 win0h;
    u16 win1h;
    u16 win0v;
    u16 win1v;
    u16 winin;
    u16 winout;
    u16 mosaic;
    u16 unused1;
    u16 bldcnt;
    u16 bldalpha;
    u16 bldy;
    u64 unused2;
    u16 unused3;

    // Sound Registers
    u16 sound1cnt_l;
    u16 sound1cnt_h;
    u16 sound1cnt_x;
    u16 unused4;
    u16 sound2cnt_l;
    u16 unused5;
    u16 sound2cnt_h;
    u16 unused6;
    u16 sound3cnt_l;
    u16 sound3cnt_h;
    u16 sound3cnt_x;
    u16 unused7;
    u16 sound4cnt_l;
    u16 unused8;
    u16 sound4cnt_h;
    u16 unused9;
    u16 soundcnt_l;
    u16 soundcnt_h;
    u16 soundcnt_x;
    u16 unused10;
    u16 soundbias;
    u32 unused11;
    u16 unused12;
    u8  _wave_ram1;
    u8  _wave_ram2;
    u8  _wave_ram3;
    u8  _wave_ram4;
    u8  _wave_ram5;
    u8  _wave_ram6;
    u8  _wave_ram7;
    u8  _wave_ram8;
    u8  _wave_ram9;
    u8  _wave_ram10;
    u8  _wave_ram11;
    u8  _wave_ram12;
    u8  _wave_ram13;
    u8  _wave_ram14;
    u8  _wave_ram15;
    u8  _wave_ram16;
    u32 fifo_a;
    u32 fifo_b;
    u64  unused13;

    // DMA Transfer Channels
    u32 dma0sad;
    u32 dma0dad;
    u16 dma0cnt_l;
    u16 dma0cnt_h;
    u32 dma1sad;
    u32 dma1dad;
    u16 dma1cnt_l;
    u16 dma1cnt_h;
    u32 dma2sad;
    u32 dma2dad;
    u16 dma2cnt_l;
    u16 dma2cnt_h;
    u32 dma3sad;
    u32 dma3dad;
    u16 dma3cnt_l;
    u16 dma3cnt_h;
    u64 unused14;
    u64 unused15;
    u64 unused16;
    u64 unused17;

    // Timer Registers
    u16 tm0cnt_l;
    u16 tm0cnt_h;
    u16 tm1cnt_l;
    u16 tm1cnt_h;
    u16 tm2cnt_l;
    u16 tm2cnt_h;
    u16 tm3cnt_l;
    u16 tm3cnt_h;
    u64 unused18;
    u64 unused19;

    // Serial Communication (1)
    // TODO: Implement SIO registers
    u64 sio0;
    u32 sio1;
    u32 unused20;

    // Keypad Input
    u16 keyinput;
    u16 keycnt;

    // Serial Communication (2)
    u16 rcnt;
    u16 ir;
    u64 unused21;
    u16 joycnt;
    u64 unused22;
    u32 unused23;
    u16 unused24;
    u32 joy_recv;
    u32 joy_trans;
    u16 joy_stat;
    u64 unused25;
    u64 unused26;
    u64 unused27;
    u64 unused28;
    u64 unused29;
    u64 unused30;
    u64 unused31;
    u64 unused32;
    u64 unused33;
    u64 unused34;
    u64 unused35;
    u64 unused36;
    u64 unused37;
    u64 unused38;
    u64 unused39;
    u64 unused40;
    u64 unused41;
    u64 unused42;
    u64 unused43;
    u64 unused44;
    u32 unused45;
    u16 unused46;

    // Interrupt, Waitstate, and Power-Down Control
    // TODO: Undocumented registers
    u16 ie;
    u16 if_;
    u16 waitcnt;
    u16 unused47;
    u16 ime;
};
#pragma pack(pop, r1)
typedef struct GBAIO_S GBAIO;
