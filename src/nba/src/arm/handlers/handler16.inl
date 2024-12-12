/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

enum class ThumbDataOp {
  AND = 0,
  EOR = 1,
  LSL = 2,
  LSR = 3,
  ASR = 4,
  ADC = 5,
  SBC = 6,
  ROR = 7,
  TST = 8,
  NEG = 9,
  CMP = 10,
  CMN = 11,
  ORR = 12,
  MUL = 13,
  BIC = 14,
  MVN = 15
};

template <int op, int imm>
void Thumb_MoveShiftedRegister(u16 instruction) {
  // THUMB.1 Move shifted register
  int dst   = (instruction >> 0) & 7;
  int src   = (instruction >> 3) & 7;
  int carry = state.cpsr.f.c;

  u32 result = state.reg[src];

  DoShift(op, result, imm, carry, true);

  state.cpsr.f.c = carry;
  state.cpsr.f.z = (result == 0);
  state.cpsr.f.n = result >> 31;

  state.reg[dst] = result;
  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;
}

template <bool immediate, bool subtract, int field3>
void Thumb_AddSub(u16 instruction) {
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;
  u32 operand = immediate ? field3 : state.reg[field3];

  if (subtract) {
    state.reg[dst] = SUB(state.reg[src], operand, true);
  } else {
    state.reg[dst] = ADD(state.reg[src], operand, true);
  }

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;
}

template <int op, int dst>
void Thumb_Op3(u16 instruction) {
  // THUMB.3 Move/compare/add/subtract immediate
  u32 imm = instruction & 0xFF;

  switch (op) {
    case 0b00:
      // MOV rD, #imm 
      state.reg[dst] = imm;
      state.cpsr.f.n = 0;
      state.cpsr.f.z = imm == 0;
      break;
    case 0b01:
      // CMP rD, #imm
      SUB(state.reg[dst], imm, true);
      break;
    case 0b10:
      // ADD rD, #imm
      state.reg[dst] = ADD(state.reg[dst], imm, true);
      break;
    case 0b11:
      // SUB rD, #imm
      state.reg[dst] = SUB(state.reg[dst], imm, true);
      break;
  }

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;
}

template <int op>
void Thumb_ALU(u16 instruction) {
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;

  switch (static_cast<ThumbDataOp>(op)) {
    case ThumbDataOp::AND: {
      state.reg[dst] &= state.reg[src];
      SetZeroAndSignFlag(state.reg[dst]);
      break;
    }
    case ThumbDataOp::EOR: {
      state.reg[dst] ^= state.reg[src];
      SetZeroAndSignFlag(state.reg[dst]);
      break;
    }
    case ThumbDataOp::LSL: {
      auto shift = state.reg[src];
      bus.Idle();
      pipe.access = Access::Code | Access::Nonsequential;

      int carry = state.cpsr.f.c;
      LSL(state.reg[dst], shift, carry);
      SetZeroAndSignFlag(state.reg[dst]);
      state.cpsr.f.c = carry;
      break;
    }
    case ThumbDataOp::LSR: {
      auto shift = state.reg[src];
      bus.Idle();
      pipe.access = Access::Code | Access::Nonsequential;

      int carry = state.cpsr.f.c;
      LSR(state.reg[dst], shift, carry, false);
      SetZeroAndSignFlag(state.reg[dst]);
      state.cpsr.f.c = carry;
      break;
    }
    case ThumbDataOp::ASR: {
      auto shift = state.reg[src];
      bus.Idle();
      pipe.access = Access::Code | Access::Nonsequential;

      int carry = state.cpsr.f.c;
      ASR(state.reg[dst], shift, carry, false);
      SetZeroAndSignFlag(state.reg[dst]);
      state.cpsr.f.c = carry;
      break;
    }
    case ThumbDataOp::ADC: {
      state.reg[dst] = ADC(state.reg[dst], state.reg[src], true);
      break;
    }
    case ThumbDataOp::SBC: {
      state.reg[dst] = SBC(state.reg[dst], state.reg[src], true);
      break;
    }
    case ThumbDataOp::ROR: {
      auto shift = state.reg[src];
      bus.Idle();
      pipe.access = Access::Code | Access::Nonsequential;      

      int carry = state.cpsr.f.c;
      ROR(state.reg[dst], shift, carry, false);
      SetZeroAndSignFlag(state.reg[dst]);
      state.cpsr.f.c = carry;
      break;
    }
    case ThumbDataOp::TST: {
      SetZeroAndSignFlag(state.reg[dst] & state.reg[src]);
      break;
    }
    case ThumbDataOp::NEG: {
      state.reg[dst] = SUB(0, state.reg[src], true);
      break;
    }
    case ThumbDataOp::CMP: {
      SUB(state.reg[dst], state.reg[src], true);
      break;
    }
    case ThumbDataOp::CMN: {
      ADD(state.reg[dst], state.reg[src], true);
      break;
    }
    case ThumbDataOp::ORR: {
      state.reg[dst] |= state.reg[src];
      SetZeroAndSignFlag(state.reg[dst]);
      break;
    }
    case ThumbDataOp::MUL: {
      u32 lhs = state.reg[src];
      u32 rhs = state.reg[dst];
      bool full = TickMultiply(rhs);
      pipe.access = Access::Code | Access::Nonsequential;

      state.reg[dst] = lhs * rhs;
      SetZeroAndSignFlag(state.reg[dst]);
      if (full) {
        state.cpsr.f.c = MultiplyCarrySimple(rhs);
      } else {
        state.cpsr.f.c = MultiplyCarryLo(lhs, rhs);
      }
      break;
    }
    case ThumbDataOp::BIC: {
      state.reg[dst] &= ~state.reg[src];
      SetZeroAndSignFlag(state.reg[dst]);
      break;
    }
    case ThumbDataOp::MVN: {
      state.reg[dst] = ~state.reg[src];
      SetZeroAndSignFlag(state.reg[dst]);
      break;
    }
  }
}

template <int op, bool high1, bool high2>
void Thumb_HighRegisterOps_BX(u16 instruction) {
  // THUMB.5 Hi register operations/branch exchange
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;

  // Instruction may access higher registers r8 - r15 ("Hi register").
  // This is archieved using two extra bits that displace the register number by 8.
  if (high1) dst |= 8;
  if (high2) src |= 8;

  u32 operand = state.reg[src];

  if (src == 15) operand &= ~1;

  if (op == 3) {
    // Branch & Exchange (BX)
    // LSB indicates new instruction set (0 = ARM, 1 = THUMB)
    if (operand & 1) {
      state.r15 = operand & ~1;
      ReloadPipeline16();
    } else {
      state.cpsr.f.thumb = 0;
      state.r15 = operand;
      ReloadPipeline32();
    }
  } else if (op == 1) {
    // Compare (CMP)
    SUB(state.reg[dst], operand, true);
    pipe.access = Access::Code | Access::Sequential;
    state.r15 += 2;
  } else {
    // Addition, move (ADD, MOV)
    if (op == 0) state.reg[dst] += operand;
    if (op == 2) state.reg[dst]  = operand;

    if (dst == 15) {
      state.r15 &= ~1;
      ReloadPipeline16();
    } else {
      pipe.access = Access::Code | Access::Sequential;
      state.r15 += 2;
    }
  }
}

template <int dst>
void Thumb_LoadStoreRelativePC(u16 instruction) {
  u32 offset  = instruction & 0xFF;
  u32 address = (state.r15 & ~2) + (offset << 2);

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  state.reg[dst] = ReadWord(address, Access::Nonsequential);
  bus.Idle();
}

template <int op, int off>
void Thumb_LoadStoreOffsetReg(u16 instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  u32 address = state.reg[base] + state.reg[off];

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  switch (op) {
    case 0b00: // STR
      WriteWord(address, state.reg[dst], Access::Nonsequential);
      break;
    case 0b01: // STRB
      WriteByte(address, (u8)state.reg[dst], Access::Nonsequential);
      break;
    case 0b10: // LDR
      state.reg[dst] = ReadWordRotate(address, Access::Nonsequential);
      bus.Idle();
      break;
    case 0b11: // LDRB
      state.reg[dst] = ReadByte(address, Access::Nonsequential);
      bus.Idle();
      break;
  }
}

template <int op, int off>
void Thumb_LoadStoreSigned(u16 instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  u32 address = state.reg[base] + state.reg[off];

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  switch (op) {
    case 0b00:
      // STRH rD, [rB, rO]
      WriteHalf(address, state.reg[dst], Access::Nonsequential);
      break;
    case 0b01:
      // LDSB rD, [rB, rO]
      state.reg[dst] = ReadByteSigned(address, Access::Nonsequential);
      bus.Idle();
      break;
    case 0b10:
      // LDRH rD, [rB, rO]
      state.reg[dst] = ReadHalfRotate(address, Access::Nonsequential);
      bus.Idle();
      break;
    case 0b11:
      // LDSH rD, [rB, rO]
      state.reg[dst] = ReadHalfSigned(address, Access::Nonsequential);
      bus.Idle();
      break;
  }
}

template <int op, int imm>
void Thumb_LoadStoreOffsetImm(u16 instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  switch (op) {
    case 0b00:
      // STR rD, [rB, #imm]
      WriteWord(state.reg[base] + imm * 4, state.reg[dst], Access::Nonsequential);
      break;
    case 0b01:
      // LDR rD, [rB, #imm]
      state.reg[dst] = ReadWordRotate(state.reg[base] + imm * 4, Access::Nonsequential);
      bus.Idle();
      break;
    case 0b10:
      // STRB rD, [rB, #imm]
      WriteByte(state.reg[base] + imm, state.reg[dst], Access::Nonsequential);
      break;
    case 0b11:
      // LDRB rD, [rB, #imm]
      state.reg[dst] = ReadByte(state.reg[base] + imm, Access::Nonsequential);
      bus.Idle();
      break;
  }
}

template <bool load, int imm>
void Thumb_LoadStoreHword(u16 instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  u32 address = state.reg[base] + imm * 2;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  if (load) {
    state.reg[dst] = ReadHalfRotate(address, Access::Nonsequential);
    bus.Idle();
  } else {
    WriteHalf(address, state.reg[dst], Access::Nonsequential);
  }
}

template <bool load, int dst>
void Thumb_LoadStoreRelativeToSP(u16 instruction) {
  u32 offset  = instruction & 0xFF;
  u32 address = state.r13 + offset * 4;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  if (load) {
    state.reg[dst] = ReadWordRotate(address, Access::Nonsequential);
    bus.Idle();
  } else {
    WriteWord(address, state.reg[dst], Access::Nonsequential);
  }
}

template <bool stackptr, int dst>
void Thumb_LoadAddress(u16 instruction) {
  u32 offset = (instruction  & 0xFF) << 2;

  if (stackptr) {
    state.reg[dst] = state.r13 + offset;
  } else {
    state.reg[dst] = (state.r15 & ~2) + offset;
  }

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;
}

template <bool sub>
void Thumb_AddOffsetToSP(u16 instruction) {
  u32 offset = (instruction  & 0x7F) * 4;

  state.r13 = state.r13 + (sub ? -offset : offset);

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 2;
}

template <bool pop, bool rbit>
void Thumb_PushPop(u16 instruction) {
  auto list = instruction & 0xFF;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  // Handle special case for empty register lists.
  if (list == 0 && !rbit) {
    if (pop) {
      state.r15 = ReadWord(state.r13, Access::Nonsequential);
      ReloadPipeline16();
      state.r13 += 0x40;
    } else {
      state.r13 -= 0x40;
      WriteWord(state.r13, state.r15, Access::Nonsequential);
    }
    return;
  }

  auto address = state.r13;
  auto access_type = Access::Nonsequential;

  if (pop) {
    for (int reg = 0; reg <= 7; reg++) {
      if (list & (1 << reg)) {
        state.reg[reg] = ReadWord(address, access_type);
        access_type = Access::Sequential;
        address += 4;
      }
    }

    if (rbit) {
      state.reg[15] = ReadWord(address, access_type) & ~1;
      state.r13 = address + 4;
      bus.Idle();
      ReloadPipeline16();
      return;
    }

    bus.Idle();
    state.r13 = address;
  } else {
    // Calculate internal start address (final r13 value)
    for (int reg = 0; reg <= 7; reg++) {
      if (list & (1 << reg))
        address -= 4;
    }

    if (rbit) address -= 4;

    // Store address in r13 before we mess with it.
    state.r13 = address;

    for (int reg = 0; reg <= 7; reg++) {
      if (list & (1 << reg)) {
        WriteWord(address, state.reg[reg], access_type);
        access_type = Access::Sequential;
        address += 4;
      }
    }

    if (rbit) {
      WriteWord(address, state.r14, access_type);
    }
  }
}

template <bool load, int base>
void Thumb_LoadStoreMultiple(u16 instruction) {
  auto list = instruction & 0xFF;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 2;

  // Handle special case for empty register lists.
  if (list == 0) {
    if (load) {
      state.r15 = ReadWord(state.reg[base], Access::Nonsequential);
      ReloadPipeline16();
    } else {
      WriteWord(state.reg[base], state.r15, Access::Nonsequential);
    }
    state.reg[base] += 0x40;
    return;
  }

  if (load) {
    u32 address = state.reg[base];
    auto access_type = Access::Nonsequential;

    for (int i = 0; i <= 7; i++) {
      if (list & (1 << i)) {
        state.reg[i] = ReadWord(address, access_type);
        access_type = Access::Sequential;
        address += 4;
      }
    }
    bus.Idle();
    if (~list & (1 << base)) {
      state.reg[base] = address;
    }
  } else {
    int count = 0;
    int first = 0;

    // Count number of registers and find first register.
    for (int reg = 7; reg >= 0; reg--) {
      if (list & (1 << reg)) {
        count++;
        first = reg;
      }
    }

    u32 address = state.reg[base];
    u32 base_new = address + count * 4;

    // Transfer first register (non-sequential access)
    WriteWord(address, state.reg[first], Access::Nonsequential);
    state.reg[base] = base_new;
    address += 4;

    // Run until end (sequential accesses)
    for (int reg = first + 1; reg <= 7; reg++) {
      if (list & (1 << reg)) {
        WriteWord(address, state.reg[reg], Access::Sequential);
        address += 4;
      }
    }
  }
}

template <int cond>
void Thumb_ConditionalBranch(u16 instruction) {
  if (CheckCondition(static_cast<Condition>(cond))) {
    u32 imm = instruction & 0xFF;

    /* Sign-extend immediate value. */
    if (imm & 0x80) {
      imm |= 0xFFFFFF00;
    }

    state.r15 += imm * 2;
    ReloadPipeline16();
  } else {
    pipe.access = Access::Code | Access::Sequential;
    state.r15 += 2;
  }
}

void Thumb_SWI(u16 instruction) {
  // Save current program status register.
  state.spsr[BANK_SVC].v = state.cpsr.v;

  // Enter SVC mode and disable IRQs.
  SwitchMode(MODE_SVC);
  state.cpsr.f.thumb = 0;
  state.cpsr.f.mask_irq = 1;

  // Save current program counter and jump to SVC exception vector.
  state.r14 = state.r15 - 2;
  state.r15 = 0x08;
  ReloadPipeline32();
}

void Thumb_UnconditionalBranch(u16 instruction) {
  u32 imm = (instruction & 0x3FF) * 2;

  // Sign-extend immediate value.
  if (instruction & 0x400) {
    imm |= 0xFFFFF800;
  }

  state.r15 += imm;
  ReloadPipeline16();
}

template <bool second_instruction>
void Thumb_LongBranchLink(u16 instruction) {
  u32 imm = instruction & 0x7FF;

  if (!second_instruction) {
    imm <<= 12;
    if (imm & 0x400000) {
      imm |= 0xFF800000;
    }
    state.r14 = state.r15 + imm;
    pipe.access = Access::Code | Access::Sequential;
    state.r15 += 2;
  } else {
    u32 temp = state.r15 - 2;

    state.r15 = (state.r14 + imm * 2) & ~1;
    state.r14 = temp | 1;
    ReloadPipeline16();
  }
}

void Thumb_Undefined(u16 instruction) { }
