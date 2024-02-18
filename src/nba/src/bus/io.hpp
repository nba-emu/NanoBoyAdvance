/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

constexpr u32 DISPCNT = 0x04000000;
constexpr u32 GREENSWAP = 0x04000002;
constexpr u32 DISPSTAT = 0x04000004;
constexpr u32 VCOUNT = 0x04000006;
constexpr u32 BG0CNT = 0x04000008;
constexpr u32 BG1CNT = 0x0400000A;
constexpr u32 BG2CNT = 0x0400000C;
constexpr u32 BG3CNT = 0x0400000E;
constexpr u32 BG0HOFS = 0x04000010;
constexpr u32 BG0VOFS = 0x04000012;
constexpr u32 BG1HOFS = 0x04000014;
constexpr u32 BG1VOFS = 0x04000016;
constexpr u32 BG2HOFS = 0x04000018;
constexpr u32 BG2VOFS = 0x0400001A;
constexpr u32 BG3HOFS = 0x0400001C;
constexpr u32 BG3VOFS = 0x0400001E;
constexpr u32 BG2PA = 0x04000020;
constexpr u32 BG2PB = 0x04000022;
constexpr u32 BG2PC = 0x04000024;
constexpr u32 BG2PD = 0x04000026;
constexpr u32 BG2X = 0x04000028;
constexpr u32 BG2Y = 0x0400002C;
constexpr u32 BG3PA = 0x04000030;
constexpr u32 BG3PB = 0x04000032;
constexpr u32 BG3PC = 0x04000034;
constexpr u32 BG3PD = 0x04000036;
constexpr u32 BG3X = 0x04000038;
constexpr u32 BG3Y = 0x0400003C;
constexpr u32 WIN0H = 0x04000040;
constexpr u32 WIN1H = 0x04000042;
constexpr u32 WIN0V = 0x04000044;
constexpr u32 WIN1V = 0x04000046;
constexpr u32 WININ = 0x04000048;
constexpr u32 WINOUT = 0x0400004A;
constexpr u32 MOSAIC = 0x0400004C;
constexpr u32 BLDCNT = 0x04000050;
constexpr u32 BLDALPHA = 0x04000052;
constexpr u32 BLDY = 0x04000054;
constexpr u32 SOUND1CNT_L = 0x04000060;
constexpr u32 SOUND1CNT_H = 0x04000062;
constexpr u32 SOUND1CNT_X = 0x04000064;
constexpr u32 SOUND2CNT_L = 0x04000068;
constexpr u32 SOUND2CNT_H = 0x0400006C;
constexpr u32 SOUND3CNT_L = 0x04000070;
constexpr u32 SOUND3CNT_H = 0x04000072;
constexpr u32 SOUND3CNT_X = 0x04000074;
constexpr u32 SOUND4CNT_L = 0x04000078;
constexpr u32 SOUND4CNT_H = 0x0400007C;
constexpr u32 SOUNDCNT_L = 0x04000080;
constexpr u32 SOUNDCNT_H = 0x04000082;
constexpr u32 SOUNDCNT_X = 0x04000084;
constexpr u32 SOUNDBIAS = 0x04000088;
constexpr u32 WAVE_RAM = 0x04000090;
constexpr u32 FIFO_A = 0x040000A0;
constexpr u32 FIFO_B = 0x040000A4;
constexpr u32 DMA0SAD = 0x040000B0;
constexpr u32 DMA0DAD = 0x040000B4;
constexpr u32 DMA0CNT_L = 0x040000B8;
constexpr u32 DMA0CNT_H = 0x040000BA;
constexpr u32 DMA1SAD = 0x040000BC;
constexpr u32 DMA1DAD = 0x040000C0;
constexpr u32 DMA1CNT_L = 0x040000C4;
constexpr u32 DMA1CNT_H = 0x040000C6;
constexpr u32 DMA2SAD = 0x040000C8;
constexpr u32 DMA2DAD = 0x040000CC;
constexpr u32 DMA2CNT_L = 0x040000D0;
constexpr u32 DMA2CNT_H = 0x040000D2;
constexpr u32 DMA3SAD = 0x040000D4;
constexpr u32 DMA3DAD = 0x040000D8;
constexpr u32 DMA3CNT_L = 0x040000DC;
constexpr u32 DMA3CNT_H = 0x040000DE;
constexpr u32 TM0CNT_L = 0x04000100;
constexpr u32 TM0CNT_H = 0x04000102;
constexpr u32 TM1CNT_L = 0x04000104;
constexpr u32 TM1CNT_H = 0x04000106;
constexpr u32 TM2CNT_L = 0x04000108;
constexpr u32 TM2CNT_H = 0x0400010A;
constexpr u32 TM3CNT_L = 0x0400010C;
constexpr u32 TM3CNT_H = 0x0400010E;
constexpr u32 SIODATA32_L = 0x04000120;
constexpr u32 SIODATA32_H = 0x04000122;
constexpr u32 SIOMULTI0 = 0x04000120;
constexpr u32 SIOMULTI1 = 0x04000122;
constexpr u32 SIOMULTI2 = 0x04000124;
constexpr u32 SIOMULTI3 = 0x04000126;
constexpr u32 SIOCNT = 0x04000128;
constexpr u32 SIOMLT_SEND = 0x0400012A;
constexpr u32 SIODATA8 = 0x0400012A;
constexpr u32 KEYINPUT = 0x04000130;
constexpr u32 KEYCNT = 0x04000132;
constexpr u32 RCNT = 0x04000134;
constexpr u32 JOYCNT = 0x04000140;
constexpr u32 JOY_RECV = 0x04000150;
constexpr u32 JOY_TRANS = 0x04000154;
constexpr u32 JOYSTAT = 0x04000158;
constexpr u32 IE = 0x04000200;
constexpr u32 IF = 0x04000202;
constexpr u32 WAITCNT = 0x04000204;
constexpr u32 IME = 0x04000208;
constexpr u32 POSTFLG = 0x04000300;
constexpr u32 HALTCNT = 0x04000301;

constexpr u32 MGBA_LOG_STRING_LO = 0x04FFF600;
constexpr u32 MGBA_LOG_STRING_HI = 0x04FFF700;
constexpr u32 MGBA_LOG_SEND = 0x04FFF700;
constexpr u32 MGBA_LOG_ENABLE = 0x04FFF780; 