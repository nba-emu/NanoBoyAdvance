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

inline void executeThumb(u32 instruction) {
    (this->*thumb_lut[instruction >> 6])(instruction);
}

template <int imm, int type>
void thumbInst1(u16 instruction);

template <bool immediate, bool subtract, int field3>
void thumbInst2(u16 instruction);

template <int op, int dst>
void thumbInst3(u16 instruction);

template <int op>
void thumbInst4(u16 instruction);

template <int op, bool high1, bool high2>
void thumbInst5(u16 instruction);

template <int dst>
void thumbInst6(u16 instruction);

template <int op, int off>
void thumbInst7(u16 instruction);

template <int op, int off>
void thumbInst8(u16 instruction);

template <int op, int imm>
void thumbInst9(u16 instruction);

template <bool load, int imm>
void thumbInst10(u16 instruction);

template <bool load, int dst>
void thumbInst11(u16 instruction);

template <bool stackptr, int dst>
void thumbInst12(u16 instruction);

template <bool sub>
void thumbInst13(u16 instruction);

template <bool pop, bool rbit>
void thumbInst14(u16 instruction);

template <bool load, int base>
void thumbInst15(u16 instruction);

template <int cond>
void thumbInst16(u16 instruction);

void thumbInst17(u16 instruction);

void thumbInst18(u16 instruction);

template <bool second_instruction>
void thumbInst19(u16 instruction);
