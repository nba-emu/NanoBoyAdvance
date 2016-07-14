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
#define KEYINPUT 0x130
#define IE 0x200
#define IF 0x202
#define WAITCNT 0x204
#define IME 0x208
#define HALTCNT 0x301

