///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#ifndef __NBA_IODEF_H__
#define __NBA_IODEF_H__


const int DISPCNT = 0x0;
const int DISPSTAT = 0x4;
const int VCOUNT = 0x6;
const int BG0CNT = 0x8;
const int BG1CNT = 0xA;
const int BG2CNT = 0xC;
const int BG3CNT = 0xE;
const int BG0HOFS = 0x10;
const int BG0VOFS = 0x12;
const int BG1HOFS = 0x14;
const int BG1VOFS = 0x16;
const int BG2HOFS = 0x18;
const int BG2VOFS = 0x1A;
const int BG3HOFS = 0x1C;
const int BG3VOFS = 0x1E;
const int BG2PA = 0x20;
const int BG2PB = 0x22;
const int BG2PC = 0x24;
const int BG2PD = 0x26;
const int BG2X = 0x28;
const int BG2Y = 0x2C;
const int BG3PA = 0x30;
const int BG3PB = 0x32;
const int BG3PC = 0x34;
const int BG3PD = 0x36;
const int BG3X = 0x38;
const int BG3Y = 0x3C;
const int WIN0H = 0x40;
const int WIN1H = 0x42;
const int WIN0V = 0x44;
const int WIN1V = 0x46;
const int WININ = 0x48;
const int WINOUT = 0x4A;
const int SOUNDBIAS = 0x88;
const int DMA0SAD = 0xB0;
const int DMA0DAD = 0xB4;
const int DMA0CNT_L = 0xB8;
const int DMA0CNT_H = 0xBA;
const int DMA1SAD = 0xBC;
const int DMA1DAD = 0xC0;
const int DMA1CNT_L = 0xC4;
const int DMA1CNT_H = 0xC6;
const int DMA2SAD = 0xC8;
const int DMA2DAD = 0xCC;
const int DMA2CNT_L = 0xD0;
const int DMA2CNT_H = 0xD2;
const int DMA3SAD = 0xD4;
const int DMA3DAD = 0xD8;
const int DMA3CNT_L = 0xDC;
const int DMA3CNT_H = 0xDE;
const int TM0CNT_L = 0x100;
const int TM0CNT_H = 0x102;
const int TM1CNT_L = 0x104;
const int TM1CNT_H = 0x106;
const int TM2CNT_L = 0x108;
const int TM2CNT_H = 0x10A;
const int TM3CNT_L = 0x10C;
const int TM3CNT_H = 0x10E;
const int KEYINPUT = 0x130;
const int IE = 0x200;
const int IF = 0x202;
const int WAITCNT = 0x204;
const int IME = 0x208;
const int HALTCNT = 0x301;


#endif  // __NBA_IODEF_H__
