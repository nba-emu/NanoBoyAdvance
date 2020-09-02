/*
 * Copyright (C) 2020 fleroviux
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
void Thumb_MoveShiftedRegister(std::uint16_t instruction) {
  // THUMB.1 Move shifted register
  int dst   = (instruction >> 0) & 7;
  int src   = (instruction >> 3) & 7;
  int carry = state.cpsr.f.c;

  std::uint32_t result = state.reg[src];

  DoShift(op, result, imm, carry, true);

  /* Update flags */
  state.cpsr.f.c = carry;
  state.cpsr.f.z = (result == 0);
  state.cpsr.f.n = result >> 31;

  state.reg[dst] = result;
  pipe.fetch_type = Access::Sequential;
  state.r15 += 2;
}

template <bool immediate, bool subtract, int field3>
void Thumb_AddSub(std::uint16_t instruction) {
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;
  std::uint32_t operand = immediate ? field3 : state.reg[field3];

  if (subtract) {
    state.reg[dst] = SUB(state.reg[src], operand, true);
  } else {
    state.reg[dst] = ADD(state.reg[src], operand, true);
  }

  pipe.fetch_type = Access::Sequential;
  state.r15 += 2;
}

template <int op, int dst>
void Thumb_Op3(std::uint16_t instruction) {
  // THUMB.3 Move/compare/add/subtract immediate
  std::uint32_t imm = instruction & 0xFF;

  switch (op) {
  case 0b00:
    /* MOV rD, #imm */
    state.reg[dst] = imm;
    state.cpsr.f.n = 0;
    state.cpsr.f.z = imm == 0;
    break;
  case 0b01:
    /* CMP rD, #imm */
    SUB(state.reg[dst], imm, true);
    break;
  case 0b10:
    /* ADD rD, #imm */
    state.reg[dst] = ADD(state.reg[dst], imm, true);
    break;
  case 0b11:
    /* SUB rD, #imm */
    state.reg[dst] = SUB(state.reg[dst], imm, true);
    break;
  }

  pipe.fetch_type = Access::Sequential;
  state.r15 += 2;
}

template <int op>
void Thumb_ALU(std::uint16_t instruction) {
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;

  pipe.fetch_type = Access::Sequential;

  /* Order of opcodes has been rearranged for readabilities sake. */
  switch (static_cast<ThumbDataOp>(op)) {
  case ThumbDataOp::MVN:
    state.reg[dst] = ~state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    break;

  /* Bitwise logical */
  case ThumbDataOp::AND:
    state.reg[dst] &= state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    break;
  case ThumbDataOp::BIC:
    state.reg[dst] &= ~state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    break;
  case ThumbDataOp::ORR:
    state.reg[dst] |= state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    break;
  case ThumbDataOp::EOR:
    state.reg[dst] ^= state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    break;
  case ThumbDataOp::TST: {
    SetZeroAndSignFlag(state.reg[dst] & state.reg[src]);
    break;
  }

  /* LSL, LSR, ASR, ROR */
  case ThumbDataOp::LSL: {
    int carry = state.cpsr.f.c;
    LSL(state.reg[dst], state.reg[src], carry);
    SetZeroAndSignFlag(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Idle();
    pipe.fetch_type = Access::Nonsequential;
    break;
  }
  case ThumbDataOp::LSR: {
    int carry = state.cpsr.f.c;
    LSR(state.reg[dst], state.reg[src], carry, false);
    SetZeroAndSignFlag(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Idle();
    pipe.fetch_type = Access::Nonsequential;
    break;
  }
  case ThumbDataOp::ASR: {
    int carry = state.cpsr.f.c;
    ASR(state.reg[dst], state.reg[src], carry, false);
    SetZeroAndSignFlag(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Idle();
    pipe.fetch_type = Access::Nonsequential;
    break;
  }
  case ThumbDataOp::ROR: {
    int carry = state.cpsr.f.c;
    ROR(state.reg[dst], state.reg[src], carry, false);
    SetZeroAndSignFlag(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Idle();
    pipe.fetch_type = Access::Nonsequential;
    break;
  }

  /* Add/Sub with carry, NEG, comparison, multiply */
  case ThumbDataOp::ADC:
    state.reg[dst] = ADC(state.reg[dst], state.reg[src], true);
    break;
  case ThumbDataOp::SBC:
    state.reg[dst] = SBC(state.reg[dst], state.reg[src], true);
    break;
  case ThumbDataOp::NEG:
    state.reg[dst] = SUB(0, state.reg[src], true);
    break;
  case ThumbDataOp::CMP:
    SUB(state.reg[dst], state.reg[src], true);
    break;
  case ThumbDataOp::CMN:
    ADD(state.reg[dst], state.reg[src], true);
    break;
  case ThumbDataOp::MUL:
    TickMultiply(state.reg[dst]);
    state.reg[dst] *= state.reg[src];
    SetZeroAndSignFlag(state.reg[dst]);
    state.cpsr.f.c = 0;
    pipe.fetch_type = Access::Nonsequential;
    break;
  }

  state.r15 += 2;
}

template <int op, bool high1, bool high2>
void Thumb_HighRegisterOps_BX(std::uint16_t instruction) {
  // THUMB.5 Hi register operations/branch exchange
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;

  // Instruction may access higher registers r8 - r15 ("Hi register").
  // This is archieved using two extra bits that displace the register number by 8.
  if (high1) dst |= 8;
  if (high2) src |= 8;

  std::uint32_t operand = state.reg[src];

  if (src == 15) operand &= ~1;

  /* Check for Branch & Exchange (bx) instruction. */
  if (op == 3) {
    /* LSB indicates new instruction set (0 = ARM, 1 = THUMB) */
    if (operand & 1) {
      state.r15 = operand & ~1;
      ReloadPipeline16();
    } else {
      state.cpsr.f.thumb = 0;
      state.r15 = operand & ~3;
      ReloadPipeline32();
    }
  /* Check for Compare (CMP) instruction. */
  } else if (op == 1) {
    SUB(state.reg[dst], operand, true);
    pipe.fetch_type = Access::Sequential;
    state.r15 += 2;
  /* Otherwise instruction is ADD or MOV. */
  } else {
    if (op == 0) state.reg[dst] += operand;
    if (op == 2) state.reg[dst]  = operand;

    if (dst == 15) {
      state.r15 &= ~1;
      ReloadPipeline16();
    } else {
      pipe.fetch_type = Access::Sequential;
      state.r15 += 2;
    }
  }
}

template <int dst>
void Thumb_LoadStoreRelativePC(std::uint16_t instruction) {
  std::uint32_t offset  = instruction & 0xFF;
  std::uint32_t address = (state.r15 & ~2) + (offset << 2);

  state.reg[dst] = ReadWord(address, Access::Nonsequential);
  interface->Idle();
  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <int op, int off>
void Thumb_LoadStoreOffsetReg(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  std::uint32_t address = state.reg[base] + state.reg[off];

  switch (op) {
  case 0b00: // STR
    WriteWord(address, state.reg[dst], Access::Nonsequential);
    break;
  case 0b01: // STRB
    WriteByte(address, (std::uint8_t)state.reg[dst], Access::Nonsequential);
    break;
  case 0b10: // LDR
    state.reg[dst] = ReadWordRotate(address, Access::Nonsequential);
    interface->Idle();
    break;
  case 0b11: // LDRB
    state.reg[dst] = ReadByte(address, Access::Nonsequential);
    interface->Idle();
    break;
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <int op, int off>
void Thumb_LoadStoreSigned(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  std::uint32_t address = state.reg[base] + state.reg[off];

  switch (op) {
  case 0b00:
    /* STRH rD, [rB, rO] */
    WriteHalf(address, state.reg[dst], Access::Nonsequential);
    break;
  case 0b01:
    /* LDSB rD, [rB, rO] */
    state.reg[dst] = ReadByteSigned(address, Access::Nonsequential);
    interface->Idle();
    break;
  case 0b10:
    /* LDRH rD, [rB, rO] */
    state.reg[dst] = ReadHalfRotate(address, Access::Nonsequential);
    interface->Idle();
    break;
  case 0b11:
    /* LDSH rD, [rB, rO] */
    state.reg[dst] = ReadHalfSigned(address, Access::Nonsequential);
    interface->Idle();
    break;
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <int op, int imm>
void Thumb_LoadStoreOffsetImm(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  switch (op) {
  case 0b00:
    /* STR rD, [rB, #imm] */
    WriteWord(state.reg[base] + imm * 4, state.reg[dst], Access::Nonsequential);
    break;
  case 0b01:
    /* LDR rD, [rB, #imm] */
    state.reg[dst] = ReadWordRotate(state.reg[base] + imm * 4, Access::Nonsequential);
    interface->Idle();
    break;
  case 0b10:
    /* STRB rD, [rB, #imm] */
    WriteByte(state.reg[base] + imm, state.reg[dst], Access::Nonsequential);
    break;
  case 0b11:
    /* LDRB rD, [rB, #imm] */
    state.reg[dst] = ReadByte(state.reg[base] + imm, Access::Nonsequential);
    interface->Idle();
    break;
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <bool load, int imm>
void Thumb_LoadStoreHword(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  std::uint32_t address = state.reg[base] + imm * 2;

  if (load) {
    state.reg[dst] = ReadHalfRotate(address, Access::Nonsequential);
    interface->Idle();
  } else {
    WriteHalf(address, state.reg[dst], Access::Nonsequential);
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <bool load, int dst>
void Thumb_LoadStoreRelativeToSP(std::uint16_t instruction) {
  std::uint32_t offset  = instruction & 0xFF;
  std::uint32_t address = state.reg[13] + offset * 4;

  if (load) {
    state.reg[dst] = ReadWordRotate(address, Access::Nonsequential);
    interface->Idle();
  } else {
    WriteWord(address, state.reg[dst], Access::Nonsequential);
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <bool stackptr, int dst>
void Thumb_LoadAddress(std::uint16_t instruction) {
  std::uint32_t offset = (instruction  & 0xFF) << 2;

  if (stackptr) {
    state.reg[dst] = state.r13 + offset;
  } else {
    state.reg[dst] = (state.r15 & ~2) + offset;
  }

  pipe.fetch_type = Access::Sequential;
  state.r15 += 2;
}

template <bool sub>
void Thumb_AddOffsetToSP(std::uint16_t instruction) {
  std::uint32_t offset = (instruction  & 0x7F) * 4;

  state.r13 += sub ? -offset : offset;
  pipe.fetch_type = Access::Sequential;
  state.r15 += 2;
}

template <bool pop, bool rbit>
void Thumb_PushPop(std::uint16_t instruction) {
  auto list = instruction & 0xFF;

  /* Handle special case for empty register lists. */
  if (list == 0 && !rbit) {
    if (pop) {
      state.r15 = ReadWord(state.r13, Access::Nonsequential);
      ReloadPipeline16();
      state.r13 += 0x40;
    } else {
      state.r13 -= 0x40;
      state.r15 += 2;
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
      state.reg[13] = address + 4;
      interface->Idle();
      ReloadPipeline16();
      return;
    }

    interface->Idle();
    state.r13 = address;
  } else {
    /* Calculate internal start address (final r13 value) */
    for (int reg = 0; reg <= 7; reg++) {
      if (list & (1 << reg))
        address -= 4;
    }

    if (rbit) address -= 4;

    /* Store address in r13 before we mess with it. */
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

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <bool load, int base>
void Thumb_LoadStoreMultiple(std::uint16_t instruction) {
  auto list = instruction & 0xFF;

  /* Handle special case for empty register lists. */
  if (list == 0) {
    if (load) {
      state.r15 = ReadWord(state.reg[base], Access::Nonsequential);
      ReloadPipeline16();
    } else {
      state.r15 += 2;
      WriteWord(state.reg[base], state.r15, Access::Nonsequential);
    }

    state.reg[base] += 0x40;
    return;
  }

  if (load) {
    std::uint32_t address = state.reg[base];
    auto access_type = Access::Nonsequential;

    for (int i = 0; i <= 7; i++) {
      if (list & (1 << i)) {
        state.reg[i] = ReadWord(address, access_type);
        access_type = Access::Sequential;
        address += 4;
      }
    }
    interface->Idle();
    if (~list & (1 << base)) {
      state.reg[base] = address;
    }
  } else {
    int count = 0;
    int first = 0;

    /* Count number of registers and find first register. */
    for (int reg = 7; reg >= 0; reg--) {
      if (list & (1 << reg)) {
        count++;
        first = reg;
      }
    }

    std::uint32_t address = state.reg[base];
    std::uint32_t base_new = address + count * 4;

    /* Transfer first register (non-sequential access) */
    WriteWord(address, state.reg[first], Access::Nonsequential);
    state.reg[base] = base_new;
    address += 4;

    /* Run until end (sequential accesses) */
    for (int reg = first + 1; reg <= 7; reg++) {
      if (list & (1 << reg)) {
        WriteWord(address, state.reg[reg], Access::Sequential);
        address += 4;
      }
    }
  }

  pipe.fetch_type = Access::Nonsequential;
  state.r15 += 2;
}

template <int cond>
void Thumb_ConditionalBranch(std::uint16_t instruction) {
  if (CheckCondition(static_cast<Condition>(cond))) {
    std::uint32_t imm = instruction & 0xFF;

    /* Sign-extend immediate value. */
    if (imm & 0x80) {
      imm |= 0xFFFFFF00;
    }

    state.r15 += imm * 2;
    ReloadPipeline16();
  } else {
    pipe.fetch_type = Access::Sequential;
    state.r15 += 2;
  }
}

void Thumb_SWI(std::uint16_t instruction) {
  /* Save return address and program status. */
  state.bank[BANK_SVC][BANK_R14] = state.r15 - 2;
  state.spsr[BANK_SVC].v = state.cpsr.v;

  /* Switch to SVC mode and disable interrupts. */
  SwitchMode(MODE_SVC);
  state.cpsr.f.thumb = 0;
  state.cpsr.f.mask_irq = 1;

  /* Jump to execution vector */
  state.r15 = 0x08;
  ReloadPipeline32();
}

void Thumb_UnconditionalBranch(std::uint16_t instruction) {
  std::uint32_t imm = (instruction & 0x3FF) * 2;

  /* Sign-extend immediate value. */
  if (instruction & 0x400) {
    imm |= 0xFFFFF800;
  }

  state.r15 += imm;
  ReloadPipeline16();
}

template <bool second_instruction>
void Thumb_LongBranchLink(std::uint16_t instruction) {
  std::uint32_t imm = instruction & 0x7FF;

  if (!second_instruction) {
    imm <<= 12;
    if (imm & 0x400000) {
      imm |= 0xFF800000;
    }
    state.r14 = state.r15 + imm;
    pipe.fetch_type = Access::Sequential;
    state.r15 += 2;
  } else {
    std::uint32_t temp = state.r15 - 2;

    state.r15 = (state.r14 + imm * 2) & ~1;
    state.r14 = temp | 1;
    ReloadPipeline16();
  }
}

void Thumb_Undefined(std::uint16_t instruction) { }
