/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

template <u16 instruction>
static constexpr auto GenerateHandlerThumb() -> Handler16 {
  // THUMB.1 Move shifted register
  if((instruction & 0xF800) < 0x1800) {
    const auto opcode = (instruction >> 11) & 3;
    const auto offset5 = (instruction >> 6) & 0x1F;

    return &ARM7TDMI::Thumb_MoveShiftedRegister<opcode, offset5>;
  }

  // THUMB.2 Add/subtract
  if((instruction & 0xF800) == 0x1800) {
    const bool immediate = (instruction >> 10) & 1;
    const bool subtract = (instruction >> 9) & 1;
    const auto field3 = (instruction >> 6) & 7;

    return &ARM7TDMI::Thumb_AddSub<immediate, subtract, field3>;
  }

  // THUMB.3 Move/compare/add/subtract immediate
  if((instruction & 0xE000) == 0x2000) {
    const auto opcode = (instruction >> 11) & 3;
    const auto rD = (instruction >> 8) & 7;

    return &ARM7TDMI::Thumb_Op3<opcode, rD>;
  }

  // THUMB.4 ALU operations
  if((instruction & 0xFC00) == 0x4000) {
    const auto opcode = (instruction >> 6) & 0xF;

    return &ARM7TDMI::Thumb_ALU<opcode>;
  }

  // THUMB.5 Hi register operations/branch exchange
  if((instruction & 0xFC00) == 0x4400) {
    const auto opcode = (instruction >> 8) & 3;
    const bool high1 = (instruction >> 7) & 1;
    const bool high2 = (instruction >> 6) & 1;

    return &ARM7TDMI::Thumb_HighRegisterOps_BX<opcode, high1, high2>;
  }

  // THUMB.6 PC-relative load
  if((instruction & 0xF800) == 0x4800) {
    const auto rD = (instruction >> 8) & 7;

    return &ARM7TDMI::Thumb_LoadStoreRelativePC<rD>;
  }

  // THUMB.7 Load/store with register offset
  if((instruction & 0xF200) == 0x5000) {
    const auto opcode = (instruction >> 10) & 3;
    const auto rO = (instruction >> 6) & 7;

    return &ARM7TDMI::Thumb_LoadStoreOffsetReg<opcode, rO>;
  }

  // THUMB.8 Load/store sign-extended byte/halfword
  if((instruction & 0xF200) == 0x5200) {
    const auto opcode = (instruction >> 10) & 3;
    const auto rO = (instruction >> 6) & 7;

    return &ARM7TDMI::Thumb_LoadStoreSigned<opcode, rO>;
  }

  // THUMB.9 Load store with immediate offset
  if((instruction & 0xE000) == 0x6000) {
    const auto opcode = (instruction >> 11) & 3;
    const auto offset5 = (instruction >> 6) & 0x1F;

    return &ARM7TDMI::Thumb_LoadStoreOffsetImm<opcode, offset5>;
  }

  // THUMB.10 Load/store halfword
  if((instruction & 0xF000) == 0x8000) {
    const bool load = (instruction >> 11) & 1;
    const auto offset5 = (instruction >> 6) & 0x1F;

    return &ARM7TDMI::Thumb_LoadStoreHword<load, offset5>;
  }

  // THUMB.11 SP-relative load/store
  if((instruction & 0xF000) == 0x9000) {
    const bool load = (instruction >> 11) & 1;
    const auto rD = (instruction >> 8) & 7;

    return &ARM7TDMI::Thumb_LoadStoreRelativeToSP<load, rD>;
  }

  // THUMB.12 Load address
  if((instruction & 0xF000) == 0xA000) {
    const bool use_r13 = (instruction >> 11) & 1;
    const auto rD = (instruction >> 8) & 7;

    return &ARM7TDMI::Thumb_LoadAddress<use_r13, rD>;
  }

  // THUMB.13 Add offset to stack pointer
  if((instruction & 0xFF00) == 0xB000) {
    const bool subtract = (instruction >> 7) & 1;

    return &ARM7TDMI::Thumb_AddOffsetToSP<subtract>;
  }

  // THUMB.14 push/pop registers
  if((instruction & 0xF600) == 0xB400) {
    const bool load = (instruction >> 11) & 1;
    const bool pc_lr = (instruction >> 8) & 1;

    return &ARM7TDMI::Thumb_PushPop<load, pc_lr>;
  }

  // THUMB.15 Multiple load/store
  if((instruction & 0xF000) == 0xC000) {
    const bool load = (instruction >> 11) & 1;
    const auto rB = (instruction >> 8) & 7;

    return &ARM7TDMI::Thumb_LoadStoreMultiple<load, rB>;
  }

  // THUMB.16 Conditional Branch
  if((instruction & 0xFF00) < 0xDF00) {
    const auto condition = (instruction >> 8) & 0xF;

    return &ARM7TDMI::Thumb_ConditionalBranch<condition>;
  }

  // THUMB.17 Software Interrupt
  if((instruction & 0xFF00) == 0xDF00) {
    return &ARM7TDMI::Thumb_SWI;
  }

  // THUMB.18 Unconditional Branch
  if((instruction & 0xF800) == 0xE000) {
    return &ARM7TDMI::Thumb_UnconditionalBranch;
  }

  // THUMB.19 Long branch with link
  if((instruction & 0xF000) == 0xF000) {
    const auto opcode = (instruction >> 11) & 1;

    return &ARM7TDMI::Thumb_LongBranchLink<opcode>;
  }

  return &ARM7TDMI::Thumb_Undefined;
}
