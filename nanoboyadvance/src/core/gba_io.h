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
    ushort dispcnt;
    ushort greenswap;
    ushort dispstat;
    ushort vcount;
    ushort bg0cnt;
    ushort bg1cnt;
    ushort bg2cnt;
    ushort bg3cnt;
    ushort bg0hofs;
    ushort bg0vofs;
    ushort bg1hofs;
    ushort bg1vofs;
    ushort bg2hofs;
    ushort bg2vofs;
    ushort bg3hofs;
    ushort bg3vofs;
    ushort bg2pa;
    ushort bg2pb;
    ushort bg2pc;
    ushort bg2pd;
    uint   bg2x;
    uint   bg2y;
    ushort bg3pa;
    ushort bg3pb;
    ushort bg3pc;
    ushort bg3pd;
    uint   bg3x;
    uint   bg3y;
    ushort win0h;
    ushort win1h;
    ushort win0v;
    ushort win1v;
    ushort winin;
    ushort winout;
    ushort mosaic;
    ushort unused1;
    ushort bldcnt;
    ushort bldalpha;
    ushort bldy;
    ulong unused2;
    ushort unused3;

    // Sound Registers
    ushort sound1cnt_l;
    ushort sound1cnt_h;
    ushort sound1cnt_x;
    ushort unused4;
    ushort sound2cnt_l;
    ushort unused5;
    ushort sound2cnt_h;
    ushort unused6;
    ushort sound3cnt_l;
    ushort sound3cnt_h;
    ushort sound3cnt_x;
    ushort unused7;
    ushort sound4cnt_l;
    ushort unused8;
    ushort sound4cnt_h;
    ushort unused9;
    ushort soundcnt_l;
    ushort soundcnt_h;
    ushort soundcnt_x;
    ushort unused10;
    ushort soundbias;
    uint   unused11;
    ushort unused12;
    ubyte  _wave_ram1;
    ubyte  _wave_ram2;
    ubyte  _wave_ram3;
    ubyte  _wave_ram4;
    ubyte  _wave_ram5;
    ubyte  _wave_ram6;
    ubyte  _wave_ram7;
    ubyte  _wave_ram8;
    ubyte  _wave_ram9;
    ubyte  _wave_ram10;
    ubyte  _wave_ram11;
    ubyte  _wave_ram12;
    ubyte  _wave_ram13;
    ubyte  _wave_ram14;
    ubyte  _wave_ram15;
    ubyte  _wave_ram16;
    uint   fifo_a;
    uint   fifo_b;
    ulong  unused13;

    // DMA Transfer Channels
    uint   dma0sad;
    uint   dma0dad;
    ushort dma0cnt_l;
    ushort dma0cnt_h;
    uint   dma1sad;
    uint   dma1dad;
    ushort dma1cnt_l;
    ushort dma1cnt_h;
    uint   dma2sad;
    uint   dma2dad;
    ushort dma2cnt_l;
    ushort dma2cnt_h;
    uint   dma3sad;
    uint   dma3dad;
    ushort dma3cnt_l;
    ushort dma3cnt_h;
    ulong  unused14;
    ulong  unused15;
    ulong  unused16;
    ulong  unused17;

    // Timer Registers
    ushort tm0cnt_l;
    ushort tm0cnt_h;
    ushort tm1cnt_l;
    ushort tm1cnt_h;
    ushort tm2cnt_l;
    ushort tm2cnt_h;
    ushort tm3cnt_l;
    ushort tm3cnt_h;
    ulong  unused18;
    ulong  unused19;

    // Serial Communication (1)
    // TODO: Implement SIO registers
    ulong  sio0;
    uint   sio1;
    uint   unused20;

    // Keypad Input
    ushort keyinput;
    ushort keycnt;

    // Serial Communication (2)
    ushort rcnt;
    ushort ir;
    ulong  unused21;
    ushort joycnt;
    ulong  unused22;
    uint   unused23;
    ushort unused24;
    uint   joy_recv;
    uint   joy_trans;
    ushort joy_stat;
    ulong  unused25;
    ulong  unused26;
    ulong  unused27;
    ulong  unused28;
    ulong  unused29;
    ulong  unused30;
    ulong  unused31;
    ulong  unused32;
    ulong  unused33;
    ulong  unused34;
    ulong  unused35;
    ulong  unused36;
    ulong  unused37;
    ulong  unused38;
    ulong  unused39;
    ulong  unused40;
    ulong  unused41;
    ulong  unused42;
    ulong  unused43;
    ulong  unused44;
    uint   unused45;
    ushort unused46;

    // Interrupt, Waitstate, and Power-Down Control
    // TODO: Undocumented registers
    ushort ie;
    ushort if_;
    ushort waitcnt;
    ushort unused47;
    ushort ime;
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
