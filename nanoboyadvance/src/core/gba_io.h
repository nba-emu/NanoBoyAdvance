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
    u32   bg2x;
    u32   bg2y;
    u16 bg3pa;
    u16 bg3pb;
    u16 bg3pc;
    u16 bg3pd;
    u32   bg3x;
    u32   bg3y;
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
    u32   unused11;
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
    u32   fifo_a;
    u32   fifo_b;
    u64  unused13;

    // DMA Transfer Channels
    u32   dma0sad;
    u32   dma0dad;
    u16 dma0cnt_l;
    u16 dma0cnt_h;
    u32   dma1sad;
    u32   dma1dad;
    u16 dma1cnt_l;
    u16 dma1cnt_h;
    u32   dma2sad;
    u32   dma2dad;
    u16 dma2cnt_l;
    u16 dma2cnt_h;
    u32   dma3sad;
    u32   dma3dad;
    u16 dma3cnt_l;
    u16 dma3cnt_h;
    u64  unused14;
    u64  unused15;
    u64  unused16;
    u64  unused17;

    // Timer Registers
    u16 tm0cnt_l;
    u16 tm0cnt_h;
    u16 tm1cnt_l;
    u16 tm1cnt_h;
    u16 tm2cnt_l;
    u16 tm2cnt_h;
    u16 tm3cnt_l;
    u16 tm3cnt_h;
    u64  unused18;
    u64  unused19;

    // Serial Communication (1)
    // TODO: Implement SIO registers
    u64  sio0;
    u32   sio1;
    u32   unused20;

    // Keypad Input
    u16 keyinput;
    u16 keycnt;

    // Serial Communication (2)
    u16 rcnt;
    u16 ir;
    u64  unused21;
    u16 joycnt;
    u64  unused22;
    u32   unused23;
    u16 unused24;
    u32   joy_recv;
    u32   joy_trans;
    u16 joy_stat;
    u64  unused25;
    u64  unused26;
    u64  unused27;
    u64  unused28;
    u64  unused29;
    u64  unused30;
    u64  unused31;
    u64  unused32;
    u64  unused33;
    u64  unused34;
    u64  unused35;
    u64  unused36;
    u64  unused37;
    u64  unused38;
    u64  unused39;
    u64  unused40;
    u64  unused41;
    u64  unused42;
    u64  unused43;
    u64  unused44;
    u32   unused45;
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

#define DMA0CNT_H 0xBA
#define DMA1CNT_H 0xC6
#define DMA2CNT_H 0xD2
#define DMA3CNT_H 0xDE
#define TM0CNT_L 0x100
#define TM0CNT_H 0x102
#define TM1CNT_L 0x104
#define TM1CNT_H 0x106
#define TM2CNT_L 0x108
#define TM2CNT_H 0x10A
#define TM3CNT_L 0x10C
#define TM3CNT_H 0x10E
#define IF 0x202
