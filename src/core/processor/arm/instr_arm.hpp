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

typedef void (ARM::*ARMInstruction)(u32);

static const ARMInstruction arm_lut[4096];

inline void executeARM(u32 instruction) {
    Condition condition = static_cast<Condition>(instruction >> 28);

    if (checkCondition(condition)) {
        int index = ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
        (this->*arm_lut[index])(instruction);
    } else {
        ctx.r15 += 4; // use constant
    }
}

template <bool immediate, int opcode, bool _set_flags, int field4>
void dataProcessingARM(u32 instruction);

template <bool immediate, bool use_spsr, bool to_status>
void statusTransferARM(u32 instruction);

template <bool accumulate, bool set_flags>
void multiplyARM(u32 instruction);

template <bool sign_extend, bool accumulate, bool set_flags>
void multiplyLongARM(u32 instruction);

template <bool swap_byte>
void singleDataSwapARM(u32 instruction);

void branchExchangeARM(u32 instruction);

template <bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode>
void halfwordSignedTransferARM(u32 instruction);

template <bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load>
void singleTransferARM(u32 instruction);

void undefinedInstARM(u32 instruction);

template <bool _pre_indexed, bool base_increment, bool user_mode, bool _write_back, bool load>
void blockTransferARM(u32 instruction);

template <bool link>
void branchARM(u32 instruction);

void swiARM(u32 instruction);
