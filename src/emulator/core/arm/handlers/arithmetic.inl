/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

void SetZeroAndSignFlag(u32 value) {
  state.cpsr.f.n = value >> 31;
  state.cpsr.f.z = (value == 0);
}

void TickMultiply(u32 multiplier) {
  u32 mask = 0xFFFFFF00;

  interface->Idle();

  while (true) {
    multiplier &= mask;
    if (multiplier == 0 || multiplier == mask) {
      break;
    }
    mask <<= 8;
    interface->Idle();
  }
}

u32 ADD(u32 op1, u32 op2, bool set_flags) {
  if (set_flags) {
    u64 result64 = (u64)op1 + (u64)op2;
    u32 result32 = (u32)result64;

    SetZeroAndSignFlag(result32);
    state.cpsr.f.c = result64 >> 32;
    state.cpsr.f.v = (~(op1 ^ op2) & (op2 ^ result32)) >> 31;
    return result32;
  } else {
    return op1 + op2;
  }
}

u32 ADC(u32 op1, u32 op2, bool set_flags) {
  if (set_flags) {
    u64 result64 = (u64)op1 + (u64)op2 + (u64)state.cpsr.f.c;
    u32 result32 = (u32)result64;

    SetZeroAndSignFlag(result32);
    state.cpsr.f.c = result64 >> 32;
    state.cpsr.f.v = (~(op1 ^ op2) & (op2 ^ result32)) >> 31;
    return result32;
  } else {
    return op1 + op2 + state.cpsr.f.c;
  }
}

u32 SUB(u32 op1, u32 op2, bool set_flags) {
  u32 result = op1 - op2;

  if (set_flags) {
    SetZeroAndSignFlag(result);
    state.cpsr.f.c = op1 >= op2;
    state.cpsr.f.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
  }

  return result;
}

u32 SBC(u32 op1, u32 op2, bool set_flags) {
  u32 op3 = (state.cpsr.f.c) ^ 1;
  u32 result = op1 - op2 - op3;

  if (set_flags) {
    SetZeroAndSignFlag(result);
    state.cpsr.f.c = (u64)op1 >= (u64)op2 + (u64)op3;
    state.cpsr.f.v = ((op1 ^ op2) & (op1 ^ result)) >> 31;
  }

  return result;
}

void DoShift(int opcode, u32& operand, u32 amount, int& carry, bool immediate) {
  amount &= 0xFF;

  switch (opcode) {
    case 0: LSL(operand, amount, carry); break;
    case 1: LSR(operand, amount, carry, immediate); break;
    case 2: ASR(operand, amount, carry, immediate); break;
    case 3: ROR(operand, amount, carry, immediate); break;
  }
}

void LSL(u32& operand, u32 amount, int& carry) {
  if (amount == 0) return;

  if (amount >= 32) {
    if (amount > 32) {
      carry = 0;
    } else {
      carry = operand & 1;
    }
    operand = 0;
    return;
  }

  carry = (operand << (amount - 1)) >> 31;
  operand <<= amount;
}

void LSR(u32& operand, u32 amount, int& carry, bool immediate) {
  if (amount == 0) {
    // LSR #0 equals to LSR #32
    if (immediate) {
      amount = 32;
    } else {
      return;
    }
  }

  if (amount >= 32) {
    if (amount > 32) {
      carry = 0;
    } else {
      carry = operand >> 31;
    }
    operand = 0;
    return;
  }

  carry = (operand >> (amount - 1)) & 1;
  operand >>= amount;
}

void ASR(u32& operand, u32 amount, int& carry, bool immediate) {
  if (amount == 0) {
    // ASR #0 equals to ASR #32
    if (immediate) {
      amount = 32;
    } else {
      return;
    }
  }

  int msb = operand >> 31;

  if (amount >= 32) {
    carry = msb;
    operand = 0xFFFFFFFF * msb;
    return;
  }

  carry = (operand >> (amount - 1)) & 1;
  operand = (operand >> amount) | ((0xFFFFFFFF * msb) << (32 - amount));
}

void ROR(u32& operand, u32 amount, int& carry, bool immediate) {
  // ROR #0 equals to RRX #1
  if (amount != 0 || !immediate) {
    if (amount == 0) return;

    amount %= 32;
    operand = (operand >> amount) | (operand << (32 - amount));
    carry = operand >> 31;
  } else {
    auto lsb = operand & 1;
    operand = (operand >> 1) | (carry << 31);
    carry = lsb;
  }
}
