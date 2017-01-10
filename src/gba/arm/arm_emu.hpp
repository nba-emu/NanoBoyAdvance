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

int arm_decode(u32 instruction);
void arm_execute(u32 instruction, int type);

void arm_data_processing(u32 instruction, bool immediate, int opcode, bool set_flags, int field4);
void arm_psr_transfer(u32 instruction, bool immediate, bool use_spsr, bool to_status);
void arm_multiply(u32 instruction, bool accumulate, bool set_flags);
void arm_multiply_long(u32 instruction, bool sign_extend, bool accumulate, bool set_flags);
void arm_single_data_swap(u32 instrution, bool swap_byte);
void arm_branch_exchange(u32 instruction);
void arm_halfword_signed_transfer(u32 instruction, bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode);
void arm_single_transfer(u32 instruction, bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load);
void arm_undefined(u32 instruction);
void arm_block_transfer(u32 instruction, bool pre_indexed, bool base_increment, bool user_mode, bool write_back, bool load);
void arm_branch(u32 instruction, bool link);
void arm_swi(u32 instruction);
