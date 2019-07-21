/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#ifndef ARM_INCLUDE_GUARD
#error "This file cannot be included regularely!"
#endif

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
  pipe.fetch_type = ACCESS_SEQ;
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

  pipe.fetch_type = ACCESS_SEQ;
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

  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 2;
}

template <int op>
void Thumb_ALU(std::uint16_t instruction) {
  int dst = (instruction >> 0) & 7;
  int src = (instruction >> 3) & 7;

  /* Order of opcodes has been rearranged for readabilities sake. */
  switch (static_cast<ThumbDataOp>(op)) {

  case ThumbDataOp::MVN:
    state.reg[dst] = ~state.reg[src];
    SetNZ(state.reg[dst]);
    break;

  /* Bitwise logical */
  case ThumbDataOp::AND:
    state.reg[dst] &= state.reg[src];
    SetNZ(state.reg[dst]);
    break;
  case ThumbDataOp::BIC:
    state.reg[dst] &= ~state.reg[src];
    SetNZ(state.reg[dst]);
    break;
  case ThumbDataOp::ORR:
    state.reg[dst] |= state.reg[src];
    SetNZ(state.reg[dst]);
    break;
  case ThumbDataOp::EOR:
    state.reg[dst] ^= state.reg[src];
    SetNZ(state.reg[dst]);
    break;
  case ThumbDataOp::TST: {
    std::uint32_t result = state.reg[dst] & state.reg[src];
    SetNZ(result);
    break;
  }

  /* LSL, LSR, ASR, ROR */
  case ThumbDataOp::LSL: {
    int carry = state.cpsr.f.c;
    LSL(state.reg[dst], state.reg[src], carry);
    SetNZ(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Tick(1);
    break;
  }
  case ThumbDataOp::LSR: {
    int carry = state.cpsr.f.c;
    LSR(state.reg[dst], state.reg[src], carry, false);
    SetNZ(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Tick(1);
    break;
  }
  case ThumbDataOp::ASR: {
    int carry = state.cpsr.f.c;
    ASR(state.reg[dst], state.reg[src], carry, false);
    SetNZ(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Tick(1);
    break;
  }
  case ThumbDataOp::ROR: {
    int carry = state.cpsr.f.c;
    ROR(state.reg[dst], state.reg[src], carry, false);
    SetNZ(state.reg[dst]);
    state.cpsr.f.c = carry;
    interface->Tick(1);
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
    /* TODO: calculate internal cycles. */
    state.reg[dst] *= state.reg[src];
    SetNZ(state.reg[dst]);
    state.cpsr.f.c = 0;
    break;

  }

  pipe.fetch_type = ACCESS_SEQ;
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
    pipe.fetch_type = ACCESS_SEQ;
    state.r15 += 2;
  /* Otherwise instruction is ADD or MOv. */
  } else {
    if (op == 0) state.reg[dst] += operand;
    if (op == 2) state.reg[dst]  = operand;

    if (dst == 15) {
      state.r15 &= ~1;
      ReloadPipeline16();
    } else {
      pipe.fetch_type = ACCESS_SEQ;
      state.r15 += 2;
    }
  }
}

template <int dst>
void Thumb_LoadStoreRelativePC(std::uint16_t instruction) {
  std::uint32_t offset  = instruction & 0xFF;
  std::uint32_t address = (state.r15 & ~2) + (offset << 2);

  state.reg[dst] = ReadWord(address, ACCESS_NSEQ);
  interface->Tick(1);
  pipe.fetch_type = ACCESS_NSEQ;
  state.r15 += 2;
}

template <int op, int off>
void Thumb_LoadStoreOffsetReg(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  std::uint32_t address = state.reg[base] + state.reg[off];

  switch (op) {
  case 0b00: // STR
    WriteWord(address, state.reg[dst], ACCESS_NSEQ);
    break;
  case 0b01: // STRB
    WriteByte(address, (std::uint8_t)state.reg[dst], ACCESS_NSEQ);
    break;
  case 0b10: // LDR
    state.reg[dst] = ReadWordRotate(address, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  case 0b11: // LDRB
    state.reg[dst] = ReadByte(address, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  }

  pipe.fetch_type = ACCESS_NSEQ;
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
    WriteHalf(address, state.reg[dst], ACCESS_NSEQ);
    break;
  case 0b01:
    /* LDSB rD, [rB, rO] */
    state.reg[dst] = ReadByteSigned(address, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  case 0b10:
    /* LDRH rD, [rB, rO] */
    state.reg[dst] = ReadHalfRotate(address, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  case 0b11:
    /* LDSH rD, [rB, rO] */
    state.reg[dst] = ReadHalfSigned(address, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  }

  pipe.fetch_type = ACCESS_NSEQ;
  state.r15 += 2;
}

template <int op, int imm>
void Thumb_LoadStoreOffsetImm(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  switch (op) {
  case 0b00:
    /* STR rD, [rB, #imm] */
    WriteWord(state.reg[base] + imm * 4, state.reg[dst], ACCESS_NSEQ);
    break;
  case 0b01:
    /* LDR rD, [rB, #imm] */
    state.reg[dst] = ReadWordRotate(state.reg[base] + imm * 4, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  case 0b10:
    /* STRB rD, [rB, #imm] */
    WriteByte(state.reg[base] + imm, state.reg[dst], ACCESS_NSEQ);
    break;
  case 0b11:
    /* LDRB rD, [rB, #imm] */
    state.reg[dst] = ReadByte(state.reg[base] + imm, ACCESS_NSEQ);
    interface->Tick(1);
    break;
  }

  pipe.fetch_type = ACCESS_NSEQ;
  state.r15 += 2;
}

template <bool load, int imm>
void Thumb_LoadStoreHword(std::uint16_t instruction) {
  int dst  = (instruction >> 0) & 7;
  int base = (instruction >> 3) & 7;

  std::uint32_t address = state.reg[base] + imm * 2;

  if (load) {
    state.reg[dst] = ReadHalfRotate(address, ACCESS_NSEQ);
    interface->Tick(1);
  } else {
    WriteHalf(address, state.reg[dst], ACCESS_NSEQ);
  }

  pipe.fetch_type = ACCESS_NSEQ;
  state.r15 += 2;
}

template <bool load, int dst>
void Thumb_LoadStoreRelativeToSP(std::uint16_t instruction) {
  std::uint32_t offset  = instruction & 0xFF;
  std::uint32_t address = state.reg[13] + offset * 4;

  if (load) {
    state.reg[dst] = ReadWordRotate(address, ACCESS_NSEQ);
    interface->Tick(1);
  } else {
    WriteWord(address, state.reg[dst], ACCESS_NSEQ);
  }

  pipe.fetch_type = ACCESS_NSEQ;
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

  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 2;
}

template <bool sub>
void Thumb_AddOffsetToSP(std::uint16_t instruction) {
  std::uint32_t offset = (instruction  & 0x7F) * 4;

  state.r13 += sub ? -offset : offset;
  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 2;
}

template <bool pop, bool rbit>
void Thumb_PushPop(std::uint16_t instruction) {
  std::uint32_t address = state.r13;

  /* TODO: handle empty register lists. */
  
  auto access_type = ACCESS_NSEQ;
  
  if (pop) {
    for (int reg = 0; reg <= 7; reg++) {
      if (instruction & (1<<reg)) {
        state.reg[reg] = ReadWord(address, access_type);
        access_type = ACCESS_SEQ;
        address += 4;
      }
    }
    
    if (rbit) {
      state.reg[15] = ReadWord(address, access_type) & ~1;
      state.reg[13] = address + 4;
      interface->Tick(1);
      ReloadPipeline16();
      return;
    }
    
    interface->Tick(1);
    state.r13 = address;
  } else {
    /* Calculate internal start address (final r13 value) */
    for (int reg = 0; reg <= 7; reg++) {
      if (instruction & (1 << reg))
        address -= 4;
    }
    if (rbit) address -= 4;

    /* Store address in r13 before we mess with it. */
    state.r13 = address;

    for (int reg = 0; reg <= 7; reg++) {
      if (instruction & (1<<reg)) {
        WriteWord(address, state.reg[reg], access_type);
        access_type = ACCESS_SEQ;
        address += 4;
      }
    }
    
    if (rbit) {
      WriteWord(address, state.r14, access_type);
    }
  }

  pipe.fetch_type = ACCESS_NSEQ;
  state.r15 += 2;
}

template <bool load, int base>
void Thumb_LoadStoreMultiple(std::uint16_t instruction) {
  /* TODO: handle empty register list. */
  
  if (load) {
    std::uint32_t address = state.reg[base];
    auto access_type = ACCESS_NSEQ;
    
    for (int i = 0; i <= 7; i++) {
      if (instruction & (1<<i)) {
        state.reg[i] = ReadWord(address, access_type);
        access_type = ACCESS_SEQ;
        address += 4;
      }
    }
    interface->Tick(1);
    if (~instruction & (1<<base)) {
      state.reg[base] = address;
    }
  } else {
    int reg = 0;

    /* First Loop - Run to first register (nonsequential access) */
    for (; reg <= 7; reg++) {
      if (instruction & (1<<reg)) {
        WriteWord(state.reg[base], state.reg[reg], ACCESS_NSEQ);
        state.reg[base] += 4;
        break;
      }
    }
    reg++;
    /* Second Loop - Run until end (sequential accesses) */
    for (; reg <= 7; reg++) {
      if (instruction & (1 << reg)) {
        WriteWord(state.reg[base], state.reg[reg], ACCESS_SEQ);
        state.reg[base] += 4;
      }
    }
  }

  pipe.fetch_type = ACCESS_NSEQ;
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
    pipe.fetch_type = ACCESS_SEQ;
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
    pipe.fetch_type = ACCESS_SEQ;
    state.r15 += 2;
  } else {
    std::uint32_t temp = state.r15 - 2;

    state.r15 = (state.r14 + imm * 2) & ~1;
    state.r14 = temp | 1;
    ReloadPipeline16();
  }
}

void Thumb_Undefined(std::uint16_t instruction) { }
