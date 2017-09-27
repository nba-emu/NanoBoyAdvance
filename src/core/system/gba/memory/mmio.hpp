/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
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

namespace Core {
    
    const int DISPCNT = 0x04000000;
    const int DISPSTAT = 0x04000004;
    const int VCOUNT = 0x04000006;
    const int BG0CNT = 0x04000008;
    const int BG1CNT = 0x0400000A;
    const int BG2CNT = 0x0400000C;
    const int BG3CNT = 0x0400000E;
    const int BG0HOFS = 0x04000010;
    const int BG0VOFS = 0x04000012;
    const int BG1HOFS = 0x04000014;
    const int BG1VOFS = 0x04000016;
    const int BG2HOFS = 0x04000018;
    const int BG2VOFS = 0x0400001A;
    const int BG3HOFS = 0x0400001C;
    const int BG3VOFS = 0x0400001E;
    const int BG2PA = 0x04000020;
    const int BG2PB = 0x04000022;
    const int BG2PC = 0x04000024;
    const int BG2PD = 0x04000026;
    const int BG2X = 0x04000028;
    const int BG2Y = 0x0400002C;
    const int BG3PA = 0x04000030;
    const int BG3PB = 0x04000032;
    const int BG3PC = 0x04000034;
    const int BG3PD = 0x04000036;
    const int BG3X = 0x04000038;
    const int BG3Y = 0x0400003C;
    const int WIN0H = 0x04000040;
    const int WIN1H = 0x04000042;
    const int WIN0V = 0x04000044;
    const int WIN1V = 0x04000046;
    const int WININ = 0x04000048;
    const int WINOUT = 0x0400004A;
    const int MOSAIC = 0x0400004C;
    const int BLDCNT = 0x04000050;
    const int BLDALPHA = 0x04000052;
    const int BLDY = 0x04000054;
    const int SOUND1CNT_L = 0x04000060;
    const int SOUND1CNT_H = 0x04000062;
    const int SOUND1CNT_X = 0x04000064;
    const int SOUND2CNT_L = 0x04000068;
    const int SOUND2CNT_H = 0x0400006C;
    const int SOUND3CNT_L = 0x04000070;
    const int SOUND3CNT_H = 0x04000072;
    const int SOUND3CNT_X = 0x04000074;
    const int SOUND4CNT_L = 0x04000078;
    const int SOUND4CNT_H = 0x0400007C;
    const int SOUNDCNT_L = 0x04000080;
    const int SOUNDCNT_H = 0x04000082;
    const int SOUNDCNT_X = 0x04000084;
    const int SOUNDBIAS = 0x04000088;
    const int WAVE_RAM = 0x04000090;
    const int FIFO_A = 0x040000A0;
    const int FIFO_B = 0x040000A4;
    const int DMA0SAD = 0x040000B0;
    const int DMA0DAD = 0x040000B4;
    const int DMA0CNT_L = 0x040000B8;
    const int DMA0CNT_H = 0x040000BA;
    const int DMA1SAD = 0x040000BC;
    const int DMA1DAD = 0x040000C0;
    const int DMA1CNT_L = 0x040000C4;
    const int DMA1CNT_H = 0x040000C6;
    const int DMA2SAD = 0x040000C8;
    const int DMA2DAD = 0x040000CC;
    const int DMA2CNT_L = 0x040000D0;
    const int DMA2CNT_H = 0x040000D2;
    const int DMA3SAD = 0x040000D4;
    const int DMA3DAD = 0x040000D8;
    const int DMA3CNT_L = 0x040000DC;
    const int DMA3CNT_H = 0x040000DE;
    const int TM0CNT_L = 0x04000100;
    const int TM0CNT_H = 0x04000102;
    const int TM1CNT_L = 0x04000104;
    const int TM1CNT_H = 0x04000106;
    const int TM2CNT_L = 0x04000108;
    const int TM2CNT_H = 0x0400010A;
    const int TM3CNT_L = 0x0400010C;
    const int TM3CNT_H = 0x0400010E;
    const int KEYINPUT = 0x04000130;
    const int IE = 0x04000200;
    const int IF = 0x04000202;
    const int WAITCNT = 0x04000204;
    const int IME = 0x04000208;
    const int HALTCNT = 0x04000301;
}
