/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

void SetZeroAndSignFlag(std::uint32_t value) {
  state.cpsr.f.n = value >> 31;
  state.cpsr.f.z = (value == 0);
}

void TickMultiply(std::uint32_t multiplier) {
  std::uint32_t mask = 0xFFFFFF00;
  
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

std::uint32_t ADD(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
  if (set_flags) {
    std::uint64_t result64 = (std::uint64_t)op1 + (std::uint64_t)op2;
    std::uint32_t result32 = (std::uint32_t)result64;

    SetZeroAndSignFlag(result32);
    state.cpsr.f.c = result64 >> 32;
    state.cpsr.f.v = (~(op1 ^ op2) & (result32 ^ op2)) >> 31;

    return result32;
  } else {
    return op1 + op2;
  }
}

std::uint32_t ADC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
  std::uint32_t op3 = state.cpsr.f.c;

  if (set_flags) {
    std::uint64_t result64 = (std::uint64_t)op1 + (std::uint64_t)op2 + (std::uint64_t)op3;
    std::uint32_t result32 = (std::uint32_t)result64;

    SetZeroAndSignFlag(result32);
    state.cpsr.f.c = result64 >> 32;
    state.cpsr.f.v = ((~(op1 ^ op2) & ((op1 + op2) ^ op2)) ^
                     (~((op1 + op2) ^ op3) & (result32 ^ op3))) >> 31;
    return result32;
  } else {
    return op1 + op2 + op3;
  }
}

std::uint32_t SUB(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
  std::uint32_t result = op1 - op2;

  if (set_flags) {
    SetZeroAndSignFlag(result);
    state.cpsr.f.c = op1 >= op2;
    state.cpsr.f.v = ((op1 ^ op2) & ~(result ^ op2)) >> 31;
  }

  return result;
}

std::uint32_t SBC(std::uint32_t op1, std::uint32_t op2, bool set_flags) {
  std::uint32_t op3 = (state.cpsr.f.c) ^ 1;

  if (set_flags) {
    std::uint32_t result1 = op1 - op2;
    std::uint32_t result2 = result1 - op3;

    SetZeroAndSignFlag(result2);
    state.cpsr.f.c = (op1 >= op2) && (result1 >= op3);
    state.cpsr.f.v = (((op1 ^ op2) & ~(result1 ^ op2)) ^ (result1 & ~result2)) >> 31;

    return result2;
  } else {
    return op1 - op2 - op3;
  }
}

void DoShift(int opcode, std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
  /* TODO: is it sane to mask the upper bits before anything else? */
  amount &= 0xFF;
  
  switch (opcode) {
    case 0: LSL(operand, amount, carry); break;
    case 1: LSR(operand, amount, carry, immediate); break;
    case 2: ASR(operand, amount, carry, immediate); break;
    case 3: ROR(operand, amount, carry, immediate); break;
  }
}

void LSL(std::uint32_t& operand, std::uint32_t amount, int& carry) {
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

void LSR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
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

void ASR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
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

void ROR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate) {
  std::uint32_t lsb;

  // ROR #0 equals to RRX #1
  if (amount != 0 || !immediate) {
    if (amount == 0) return;

    amount %= 32;
    operand = (operand >> (amount - 1)) | (operand << (32 - amount + 1));
    carry = operand & 1;
    operand = (operand >> 1) | (operand << 31);
  } else {
    lsb = operand & 1;
    operand = (operand >> 1) | (carry << 31);
    carry = lsb;
  }
}
