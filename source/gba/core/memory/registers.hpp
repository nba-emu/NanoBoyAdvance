/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

constexpr int DISPCNT = 0x04000000;
constexpr int DISPSTAT = 0x04000004;
constexpr int VCOUNT = 0x04000006;
constexpr int BG0CNT = 0x04000008;
constexpr int BG1CNT = 0x0400000A;
constexpr int BG2CNT = 0x0400000C;
constexpr int BG3CNT = 0x0400000E;
constexpr int BG0HOFS = 0x04000010;
constexpr int BG0VOFS = 0x04000012;
constexpr int BG1HOFS = 0x04000014;
constexpr int BG1VOFS = 0x04000016;
constexpr int BG2HOFS = 0x04000018;
constexpr int BG2VOFS = 0x0400001A;
constexpr int BG3HOFS = 0x0400001C;
constexpr int BG3VOFS = 0x0400001E;
constexpr int BG2PA = 0x04000020;
constexpr int BG2PB = 0x04000022;
constexpr int BG2PC = 0x04000024;
constexpr int BG2PD = 0x04000026;
constexpr int BG2X = 0x04000028;
constexpr int BG2Y = 0x0400002C;
constexpr int BG3PA = 0x04000030;
constexpr int BG3PB = 0x04000032;
constexpr int BG3PC = 0x04000034;
constexpr int BG3PD = 0x04000036;
constexpr int BG3X = 0x04000038;
constexpr int BG3Y = 0x0400003C;
constexpr int WIN0H = 0x04000040;
constexpr int WIN1H = 0x04000042;
constexpr int WIN0V = 0x04000044;
constexpr int WIN1V = 0x04000046;
constexpr int WININ = 0x04000048;
constexpr int WINOUT = 0x0400004A;
constexpr int MOSAIC = 0x0400004C;
constexpr int BLDCNT = 0x04000050;
constexpr int BLDALPHA = 0x04000052;
constexpr int BLDY = 0x04000054;
constexpr int SOUND1CNT_L = 0x04000060;
constexpr int SOUND1CNT_H = 0x04000062;
constexpr int SOUND1CNT_X = 0x04000064;
constexpr int SOUND2CNT_L = 0x04000068;
constexpr int SOUND2CNT_H = 0x0400006C;
constexpr int SOUND3CNT_L = 0x04000070;
constexpr int SOUND3CNT_H = 0x04000072;
constexpr int SOUND3CNT_X = 0x04000074;
constexpr int SOUND4CNT_L = 0x04000078;
constexpr int SOUND4CNT_H = 0x0400007C;
constexpr int SOUNDCNT_L = 0x04000080;
constexpr int SOUNDCNT_H = 0x04000082;
constexpr int SOUNDCNT_X = 0x04000084;
constexpr int SOUNDBIAS = 0x04000088;
constexpr int WAVE_RAM = 0x04000090;
constexpr int FIFO_A = 0x040000A0;
constexpr int FIFO_B = 0x040000A4;
constexpr int DMA0SAD = 0x040000B0;
constexpr int DMA0DAD = 0x040000B4;
constexpr int DMA0CNT_L = 0x040000B8;
constexpr int DMA0CNT_H = 0x040000BA;
constexpr int DMA1SAD = 0x040000BC;
constexpr int DMA1DAD = 0x040000C0;
constexpr int DMA1CNT_L = 0x040000C4;
constexpr int DMA1CNT_H = 0x040000C6;
constexpr int DMA2SAD = 0x040000C8;
constexpr int DMA2DAD = 0x040000CC;
constexpr int DMA2CNT_L = 0x040000D0;
constexpr int DMA2CNT_H = 0x040000D2;
constexpr int DMA3SAD = 0x040000D4;
constexpr int DMA3DAD = 0x040000D8;
constexpr int DMA3CNT_L = 0x040000DC;
constexpr int DMA3CNT_H = 0x040000DE;
constexpr int TM0CNT_L = 0x04000100;
constexpr int TM0CNT_H = 0x04000102;
constexpr int TM1CNT_L = 0x04000104;
constexpr int TM1CNT_H = 0x04000106;
constexpr int TM2CNT_L = 0x04000108;
constexpr int TM2CNT_H = 0x0400010A;
constexpr int TM3CNT_L = 0x0400010C;
constexpr int TM3CNT_H = 0x0400010E;
constexpr int KEYINPUT = 0x04000130;
constexpr int RCNT = 0x04000134;
constexpr int IE = 0x04000200;
constexpr int IF = 0x04000202;
constexpr int WAITCNT = 0x04000204;
constexpr int IME = 0x04000208;
constexpr int HALTCNT = 0x04000301;
