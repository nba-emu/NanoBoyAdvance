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

typedef void (ARM::*ThumbInstruction)(u16);

static const ThumbInstruction thumb_lut[1024];

inline void thumb_execute(u32 instruction) 
{
  (this->*thumb_lut[instruction >> 6])(instruction);
}

template <int imm, int type>
void thumb_1(u16 instruction);

template <bool immediate, bool subtract, int field3>
void thumb_2(u16 instruction);

template <int op, int dst>
void thumb_3(u16 instruction);

template <int op>
void thumb_4(u16 instruction);

template <int op, bool high1, bool high2>
void thumb_5(u16 instruction);

template <int dst>
void thumb_6(u16 instruction);

template <int op, int off>
void thumb_7(u16 instruction);

template <int op, int off>
void thumb_8(u16 instruction);

template <int op, int imm>
void thumb_9(u16 instruction);

template <bool load, int imm>
void thumb_10(u16 instruction);

template <bool load, int dst>
void thumb_11(u16 instruction);

template <bool stackptr, int dst>
void thumb_12(u16 instruction);

template <bool sub>
void thumb_13(u16 instruction);

template <bool pop, bool rbit>
void thumb_14(u16 instruction);

template <bool load, int base>
void thumb_15(u16 instruction);

template <int cond>
void thumb_16(u16 instruction);

void thumb_17(u16 instruction);

void thumb_18(u16 instruction);

template <bool second_instruction>
void thumb_19(u16 instruction);

