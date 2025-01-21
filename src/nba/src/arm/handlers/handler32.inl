/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

enum class DataOp {
  AND = 0,
  EOR = 1,
  SUB = 2,
  RSB = 3,
  ADD = 4,
  ADC = 5,
  SBC = 6,
  RSC = 7,
  TST = 8,
  TEQ = 9,
  CMP = 10,
  CMN = 11,
  ORR = 12,
  MOV = 13,
  BIC = 14,
  MVN = 15
};

template <bool immediate, DataOp opcode, bool set_flags, int field4>
void ARM_DataProcessing(u32 instruction) {  
  constexpr int  shift_type = ( field4 >> 1) & 3;
  constexpr bool shift_imm  = (~field4 >> 0) & 1;

  int reg_dst = (instruction >> 12) & 0xF;
  int reg_op1 = (instruction >> 16) & 0xF;
  int reg_op2 = (instruction >>  0) & 0xF;

  int carry = state.cpsr.f.c;
  u32 op1;
  u32 op2;

  pipe.access = Access::Code | Access::Sequential;

  if constexpr (immediate) {
    int value = instruction & 0xFF;
    int shift = ((instruction >> 8) & 0xF) * 2;

    if (shift != 0) {
      carry = (value >> (shift - 1)) & 1;
      op2   = (value >> shift) | (value << (32 - shift));
    } else {
      op2 = value;
    }

    op1 = GetReg(reg_op1);
  } else {
    u32 shift;

    if constexpr (shift_imm) {
      shift = (instruction >> 7) & 0x1F;
    } else {
      shift = GetReg((instruction >> 8) & 0xF);
      state.r15 += 4;
      bus.Idle();

      pipe.access = Access::Code | Access::Nonsequential;
    }

    op1 = GetReg(reg_op1);
    op2 = GetReg(reg_op2);

    DoShift(shift_type, op2, shift, carry, shift_imm);
  }

  auto& cpsr = state.cpsr;
  u32 result;

  switch (opcode) {
    case DataOp::AND:
      result = op1 & op2;
      if constexpr (set_flags) {
        SetZeroAndSignFlag(result);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, result);
      break;
    case DataOp::EOR:
      result = op1 ^ op2;
      if constexpr (set_flags) {
        SetZeroAndSignFlag(result);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, result);
      break;
    case DataOp::SUB:
      SetReg(reg_dst, SUB(op1, op2, set_flags));
      break;
    case DataOp::RSB:
      SetReg(reg_dst, SUB(op2, op1, set_flags));
      break;
    case DataOp::ADD:
      SetReg(reg_dst, ADD(op1, op2, set_flags));
      break;
    case DataOp::ADC:
      SetReg(reg_dst, ADC(op1, op2, set_flags));
      break;
    case DataOp::SBC:
      SetReg(reg_dst, SBC(op1, op2, set_flags));
      break;
    case DataOp::RSC:
      SetReg(reg_dst, SBC(op2, op1, set_flags));
      break;
    case DataOp::TST:
      SetZeroAndSignFlag(op1 & op2);
      cpsr.f.c = carry;
      break;
    case DataOp::TEQ:
      SetZeroAndSignFlag(op1 ^ op2);
      cpsr.f.c = carry;
      break;
    case DataOp::CMP:
      SUB(op1, op2, true);
      break;
    case DataOp::CMN:
      ADD(op1, op2, true);
      break;
    case DataOp::ORR:
      result = op1 | op2;
      if (set_flags) {
        SetZeroAndSignFlag(result);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, result);
      break;
    case DataOp::MOV:
      if constexpr (set_flags) {
        SetZeroAndSignFlag(op2);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, op2);
      break;
    case DataOp::BIC:
      result = op1 & ~op2;
      if constexpr (set_flags) {
        SetZeroAndSignFlag(result);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, result);
      break;
    case DataOp::MVN:
      result = ~op2;
      if constexpr (set_flags) {
        SetZeroAndSignFlag(result);
        cpsr.f.c = carry;
      }
      SetReg(reg_dst, result);
      break;
  }

  if (reg_dst == 15) {
    if constexpr (set_flags) {
      auto spsr = GetSPSR();

      SwitchMode(spsr.f.mode);
      state.cpsr.v = spsr.v;
    }

    if constexpr (opcode != DataOp::TST &&
                  opcode != DataOp::TEQ &&
                  opcode != DataOp::CMP &&
                  opcode != DataOp::CMN) {
      if (state.cpsr.f.thumb) {
        ReloadPipeline16();
      } else {
        ReloadPipeline32();
      }
    } else if constexpr (immediate || shift_imm) {
      state.r15 += 4;
    }
  } else if constexpr (immediate || shift_imm) {
    state.r15 += 4;
  }
}

template <bool immediate, bool use_spsr, bool to_status>
void ARM_StatusTransfer(u32 instruction) {
  if (to_status) {
    u32 op;
    u32 mask = 0;

    // Create mask based on fsxc-bits.
    if (instruction & (1 << 16)) mask |= 0x000000FF;
    if (instruction & (1 << 17)) mask |= 0x0000FF00;
    if (instruction & (1 << 18)) mask |= 0x00FF0000;
    if (instruction & (1 << 19)) mask |= 0xFF000000;

    // Decode source operand. 
    if (immediate) {
      int value = instruction & 0xFF;
      int shift = ((instruction >> 8) & 0xF) * 2;

      op = (value >> shift) | (value << (32 - shift));
    } else {
      op = GetReg(instruction & 0xF);
    }

    // Apply masked replace to SPSR or CPSR.
    if (!use_spsr) {
      // In non-privileged mode (user mode): only condition code bits of CPSR can be changed, control bits can't.
      if (state.cpsr.f.mode == MODE_USR) {
        mask &= 0xFF000000;
      }
      if (mask & 0xFF) {
        // CPSR/SPSR bit4 is forced to one on the ARM7TDMI:
        op |= 0x00000010;

        SwitchMode(static_cast<Mode>(op & 0x1F));
      }
      // TODO: handle code that alters the Thumb-bit.
      state.cpsr.v = (state.cpsr.v & ~mask) | (op & mask);
    } else if (p_spsr != &state.cpsr && likely(!cpu_mode_is_invalid)) {
      p_spsr->v = (GetSPSR().v & ~mask) | (op & mask);
    }
  } else {
    int dst = (instruction >> 12) & 0xF;

    if (use_spsr) {
      SetReg(dst, GetSPSR().v);
    } else {
      SetReg(dst, state.cpsr.v);
    }
  }

  pipe.access = Access::Code | Access::Sequential;
  state.r15 += 4;
}

template <bool accumulate, bool set_flags>
void ARM_Multiply(u32 instruction) {
  int op1 = (instruction >>  0) & 0xF;
  int op2 = (instruction >>  8) & 0xF;
  int op3 = (instruction >> 12) & 0xF;
  int dst = (instruction >> 16) & 0xF;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  auto lhs = GetReg(op1);
  auto rhs = GetReg(op2);
  auto result = lhs * rhs;

  bool full = TickMultiply(rhs);

  u32 accum = 0;
  if (accumulate) {
    accum = GetReg(op3);
    result += accum;
    bus.Idle();
  }

  if (set_flags) {
    SetZeroAndSignFlag(result);
    if (full) {
      state.cpsr.f.c = MultiplyCarrySimple(rhs);
    } else {
      state.cpsr.f.c = MultiplyCarryLo(lhs, rhs, accum);
    }
  }

  SetReg(dst, result);

  if (dst == 15) {
    ReloadPipeline32();
  }
}

template <bool sign_extend, bool accumulate, bool set_flags>
void ARM_MultiplyLong(u32 instruction) {
  int op1 = (instruction >> 0) & 0xF;
  int op2 = (instruction >> 8) & 0xF;

  int dst_lo = (instruction >> 12) & 0xF;
  int dst_hi = (instruction >> 16) & 0xF;

  u64 result;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  auto lhs = GetReg(op1);
  auto rhs = GetReg(op2);

  if (sign_extend) {
    result = s64(s32(lhs)) * s64(s32(rhs));
  } else {
    result = u64(lhs) * u64(rhs);
  }

  bool full = TickMultiply<sign_extend>(rhs);
  bus.Idle();

  u32 accum_lo = 0;
  u32 accum_hi = 0;
  if (accumulate) {
    accum_lo = GetReg(dst_lo);
    accum_hi = GetReg(dst_hi);

    u64 value = accum_hi;
    value <<= 32;
    value |= accum_lo;

    result += value;
    bus.Idle();
  }

  u32 result_hi = result >> 32;

  if (set_flags) {
    state.cpsr.f.n = result_hi >> 31;
    state.cpsr.f.z = result == 0;
    if (full) {
      state.cpsr.f.c = MultiplyCarryHi<sign_extend>(lhs, rhs, accum_hi);
    } else {
      state.cpsr.f.c = MultiplyCarryLo(lhs, rhs, accum_lo);
    }
  }

  SetReg(dst_lo, result & 0xFFFFFFFF);
  SetReg(dst_hi, result_hi);

  if (dst_lo == 15 || dst_hi == 15) {
    ReloadPipeline32();
  }
}

template <bool byte>
void ARM_SingleDataSwap(u32 instruction) {
  int src  = (instruction >>  0) & 0xF;
  int dst  = (instruction >> 12) & 0xF;
  int base = (instruction >> 16) & 0xF;

  u32 tmp;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  if (byte) {
    tmp = ReadByte(GetReg(base), Access::Nonsequential);
    WriteByte(GetReg(base), (u8)GetReg(src), Access::Nonsequential | Access::Lock);
  } else {
    tmp = ReadWordRotate(GetReg(base), Access::Nonsequential);
    WriteWord(GetReg(base), GetReg(src), Access::Nonsequential | Access::Lock);
  }

  bus.Idle();
  
  SetReg(dst, tmp);

  if (dst == 15) {
    ReloadPipeline32();
  }
}

void ARM_BranchAndExchange(u32 instruction) {
  u32 address = GetReg(instruction & 0xF);

  if (address & 1) {
    state.r15 = address & ~1;
    state.cpsr.f.thumb = 1;
    ReloadPipeline16();
  } else {
    state.r15 = address;
    ReloadPipeline32();
  }
}

template <bool pre, bool add, bool immediate, bool writeback, bool load, int opcode>
void ARM_HalfwordSignedTransfer(u32 instruction) {
  int dst  = (instruction >> 12) & 0xF;
  int base = (instruction >> 16) & 0xF;

  u32 offset;
  u32 address = GetReg(base);

  if constexpr (immediate) {
    offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
  } else {
    offset = GetReg(instruction & 0xF);
  }

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  if constexpr (!add) {
    offset = -offset;
  }

  if constexpr (pre) {
    address += offset;
  }

  switch (opcode) {
    case 0: break;
    case 1:
      if (load) {
        auto value = ReadHalfRotate(address, Access::Nonsequential);
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
        bus.Idle();
        SetReg(dst, value);
      } else {
        WriteHalf(address, GetReg(dst), Access::Nonsequential);
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
      }
      break;
    case 2:
      if (load) {
        auto value = ReadByteSigned(address, Access::Nonsequential);
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
        bus.Idle();
        SetReg(dst, value);
      } else {
        // ARMv5 LDRD: this opcode is unpredictable on ARMv4T.
        // On ARM7TDMI-S it doesn't seem to perform any memory access,
        // so the load/store cycle probably is internal in this case.
        bus.Idle();
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
        bus.Idle();
      }
      break;
    case 3:
      if (load) {
        auto value = ReadHalfSigned(address, Access::Nonsequential);
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
        bus.Idle();
        SetReg(dst, value);
      } else {
        // ARMv5 STRD: this opcode is unpredictable on ARMv4T.
        // On ARM7TDMI-S it doesn't seem to perform any memory access,
        // so the load/store cycle probably is internal in this case.
        bus.Idle();
        if constexpr (writeback || !pre) {
          SetReg(base, GetReg(base) + offset);
        }
      }
      break;
  }

  if (load && dst == 15) {
    ReloadPipeline32();
  }
}

template <bool link>
void ARM_BranchAndLink(u32 instruction) {
  u32 offset = instruction & 0xFFFFFF;

  if (offset & 0x800000) {
    offset |= 0xFF000000;
  }

  if (link) {
    SetReg(14, state.r15 - 4);
  }

  state.r15 += offset * 4;
  ReloadPipeline32();
}

template <bool immediate, bool pre, bool add, bool byte, bool writeback, bool load>
void ARM_SingleDataTransfer(u32 instruction) {
  u32 offset;

  int dst  = (instruction >> 12) & 0xF;
  int base = (instruction >> 16) & 0xF;
  u32 address = GetReg(base);

  // Calculate offset relative to base register.
  if constexpr (immediate) {
    offset = instruction & 0xFFF;
  } else {
    int carry  = state.cpsr.f.c;
    int opcode = (instruction >> 5) & 3;
    int amount = (instruction >> 7) & 0x1F;

    offset = GetReg(instruction & 0xF);
    DoShift(opcode, offset, amount, carry, true);
  }

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  if constexpr(!add) {
    offset = -offset;
  }

  if constexpr (pre) {
    address += offset;
  }

  if constexpr (load) {
    u32 value;

    if constexpr (byte) {
      value = ReadByte(address, Access::Nonsequential);
    } else {
      value = ReadWordRotate(address, Access::Nonsequential);
    }

    if constexpr (writeback || !pre) {
      SetReg(base, GetReg(base) + offset);
    }

    bus.Idle();

    SetReg(dst, value);
  } else {
    if constexpr (byte) {
      WriteByte(address, (u8)GetReg(dst), Access::Nonsequential);
    } else {
      WriteWord(address, GetReg(dst), Access::Nonsequential);
    }

    if constexpr (writeback || !pre) {
      SetReg(base, GetReg(base) + offset);
    }
  }

  if (load && dst == 15) {
    ReloadPipeline32();
  }
}

template <bool _pre, bool add, bool user_mode, bool writeback, bool load>
void ARM_BlockDataTransfer(u32 instruction) {
  // TODO: test special case with usermode registers and a banked base register.
  int base = (instruction >> 16) & 0xF;
  int list = instruction & 0xFFFF;

  Mode mode = state.cpsr.f.mode;
  bool transfer_pc = list & (1 << 15);
  int  first = 0;
  int  bytes = 0;
  bool pre = _pre;

  u32 address = GetReg(base);

  if (list != 0) {
    // Determine number of bytes to transfer and the first register in the list.
    for (int i = 15; i >= 0; i--) {
      if (~list & (1 << i)) {
        continue;
      }
      first = i;
      bytes += 4;
    }
  } else {
    /* If the register list is empty, only r15 will be loaded/stored but
     * the base will be incremented/decremented as if each register was transferred.
     */
    list  = 1 << 15;
    first = 15;
    transfer_pc = true;
    bytes = 64;
  }

  /**
   * Whether the instruction is a LDM^ instruction and we need to switch to user mode.
   * Note that we only switch to user mode if we aren't in user or system mode already.
   * This is important for emulation of the LDM user mode register bus conflict,
   * since the implementation of this quirk in GetReg() and SetReg() only works if the CPU isn't in user or system mode anymore.
   */
  const bool switch_mode = user_mode && (!load || !transfer_pc) && mode != MODE_USR && mode != MODE_SYS;

  if (switch_mode) {
    SwitchMode(MODE_USR);
  }

  u32 base_new = address;
  u32 base_old = address;

  /* The CPU always transfers registers sequentially,
   * for decrementing modes it determines the final address first and
   * then transfers the registers with incrementing addresses.
   * Due to the inverted order post-decrement becomes pre-increment and
   * pre-decrement becomes post-increment.
   */
  if constexpr (!add) {
    pre = !pre;
    address  -= bytes;
    base_new -= bytes;
  } else {
    base_new += bytes;
  }

  auto access_type = Access::Nonsequential;

  pipe.access = Access::Code | Access::Nonsequential;
  state.r15 += 4;

  for (int i = first; i < 16; i++) {
    if (~list & (1 << i)) {
      continue;
    }

    if (pre) {
      address += 4;
    }

    if constexpr (load) {
      auto value = ReadWord(address, access_type);
      if (writeback && i == first) {
        SetReg(base, base_new);
      }
      SetReg(i, value);
    } else {
      WriteWord(address, GetReg(i), access_type);
      if (writeback && i == first) {
        SetReg(base, base_new);
      }
    }

    if (!pre) {
      address += 4;
    }

    access_type = Access::Sequential;
  }

  if constexpr (load) {
    bus.Idle();

    if (switch_mode) {
      /* During the following two cycles of a user mode LDM,
       * register accesses will go to both the user bank and original bank.
       */
      ldm_usermode_conflict = true;
      scheduler.Add(2, Scheduler::EventClass::ARM_ldm_usermode_conflict);
    }

    if (transfer_pc) {
      if constexpr (user_mode) {
        auto spsr = GetSPSR();
        SwitchMode(spsr.f.mode);
        state.cpsr.v = spsr.v;
      }

      if (state.cpsr.f.thumb) {
        ReloadPipeline16();
      } else {
        ReloadPipeline32();
      }
    }
  }

  if (switch_mode) {
    SwitchMode(mode);
  }
}

void ARM_Undefined(u32 instruction) {
  // Save current program status register.
  state.spsr[BANK_UND].v = state.cpsr.v;

  // Enter UND mode and disable IRQs.
  SwitchMode(MODE_UND);
  state.cpsr.f.mask_irq = 1;

  // Save current program counter and jump to UND exception vector.
  SetReg(14, state.r15 - 4);
  state.r15 = 0x04;
  ReloadPipeline32();
}

void ARM_SWI(u32 instruction) {
  // Save current program status register.
  state.spsr[BANK_SVC].v = state.cpsr.v;

  // Enter SVC mode and disable IRQs.
  SwitchMode(MODE_SVC);
  state.cpsr.f.mask_irq = 1;

  // Save current program counter and jump to SVC exception vector.
  SetReg(14, state.r15 - 4);
  state.r15 = 0x08;
  ReloadPipeline32();
}
