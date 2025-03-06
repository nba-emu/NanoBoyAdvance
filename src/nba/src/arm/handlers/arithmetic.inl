/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 *
 * Multiplication carry flag algorithm has been altered from its original form according to its GPL-compatible license, as follows:
 *
 * Copyright (C) 2024 zaydlang, calc84maniac
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *   3. This notice may not be removed or altered from any source distribution.
 */

void SetZeroAndSignFlag(u32 value) {
  state.cpsr.f.n = value >> 31;
  state.cpsr.f.z = (value == 0);
}

template<bool is_signed = true>
bool TickMultiply(u32 multiplier) {
  u32 mask = 0xFFFFFF00;

  bus.Idle();

  while (true) {
    multiplier &= mask;

    if (multiplier == 0) break;

    if constexpr(is_signed) {
      if (multiplier == mask) break;
    }

    mask <<= 8;
    bus.Idle();
  }

  // Return true if full ticks used.
  return mask == 0;
}

bool MultiplyCarrySimple(u32 multiplier) {
  // Carry comes directly from final injected booth carry bit.
  // Final booth addend is negative only if upper 2 bits are 10.
  return (multiplier >> 30) == 2;
}

bool MultiplyCarryLo(u32 multiplicand, u32 multiplier, u32 accum = 0) {
  // Set low bit of multiplicand to cause negation to invert the upper bits.
  // This bit cannot propagate to the resulting carry bit.
  multiplicand |= 1;

  // Optimized first iteration.
  u32 booth = (s32)(multiplier << 31) >> 31;
  u32 carry = multiplicand * booth;
  u32 sum = carry + accum;

  int shift = 29;
  do {
    // Process 8 multiplier bits using 4 booth iterations.
    for (int i = 0; i < 4; i++, shift -= 2) {
      // Get next booth factor (-2 to 2, shifted left by 30-shift).
      u32 next_booth = (s32)(multiplier << shift) >> shift;
      u32 factor = next_booth - booth;
      booth = next_booth;
      // Get scaled value of booth addend.
      u32 addend = multiplicand * factor;
      // Accumulate addend with carry-save add.
      accum ^= carry ^ addend;
      sum += addend;
      carry = sum - accum;
    }
  } while (booth != multiplier);

  // Carry flag comes from bit 31 of carry-save adder's final carry.
  return carry >> 31;
}

template<bool sign_extend>
bool MultiplyCarryHi(u32 multiplicand, u32 multiplier, u32 accum_hi = 0) {
  // Only last 3 booth iterations are relevant to output carry.
  // Reduce scale of both inputs to get upper bits of 64-bit booth addends
  // in upper bits of 32-bit values, while handling sign extension.
  if (sign_extend) {
    multiplicand = (s32)multiplicand >> 6;
    multiplier = (s32)multiplier >> 26;
  } else {
    multiplicand >>= 6;
    multiplier >>= 26;
  }
  // Set low bit of multiplicand to cause negation to invert the upper bits.
  // This bit cannot propagate to the resulting carry bit.
  multiplicand |= 1;

  // Pre-populate magic bit 61 for carry.
  u32 carry = ~accum_hi & 0x20000000;
  // Pre-populate magic bits 63-60 for accum (with carry magic pre-added).
  u32 accum = accum_hi - 0x08000000;

  // Get factors for last 3 booth iterations.
  u32 booth0 = (s32)(multiplier << 27) >> 27;
  u32 booth1 = (s32)(multiplier << 29) >> 29;
  u32 booth2 = (s32)(multiplier << 31) >> 31;
  u32 factor0 = multiplier - booth0;
  u32 factor1 = booth0 - booth1;
  u32 factor2 = booth1 - booth2;

  // Get scaled value of 3rd-last booth addend.
  u32 addend = multiplicand * factor2;
  // Finalize bits 61-60 of accum magic using its sign.
  accum -= addend & 0x10000000;
  // Get scaled value of 2nd-last booth addend.
  addend = multiplicand * factor1;
  // Finalize bits 63-62 of accum magic using its sign.
  accum -= addend & 0x40000000;

  // Get carry from carry-save add in bit 61 and propagate it to bit 62.
  u32 sum = accum + (addend & 0x20000000);
  // Subtract out carry magic to get actual accum magic.
  accum -= carry;

  // Get scaled value of last booth addend.
  addend = multiplicand * factor0;
  // Add to bit 62 and propagate carry.
  sum += addend & 0x40000000;

  // Cancel out accum magic bit 63 to get carry bit 63.
  return (sum ^ accum) >> 31;
}

u32 ADD(u32 op1, u32 op2, bool set_flags) {
  u32 result = op1 + op2;

  if (set_flags) {
    SetZeroAndSignFlag(result);
    state.cpsr.f.c = result < op1;
    state.cpsr.f.v = (~(op1 ^ op2) & (op2 ^ result)) >> 31;
  }

  return result;
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

void DoShift(int opcode, u32& operand, u8 amount, int& carry, bool immediate) {
  switch (opcode) {
    case 0: LSL(operand, amount, carry); break;
    case 1: LSR(operand, amount, carry, immediate); break;
    case 2: ASR(operand, amount, carry, immediate); break;
    case 3: ROR(operand, amount, carry, immediate); break;
  }
}

void LSL(u32& operand, u8 amount, int& carry) {
  const int adj_amount = std::min<int>(amount, 33);
  const u32 result = (u32)((u64)operand << adj_amount);
  if(adj_amount != 0) {
    carry = (u32)((u64)operand << (adj_amount - 1)) >> 31;
  }
  operand = result;
}

void LSR(u32& operand, u8 amount, int& carry, bool immediate) {
  // LSR #32 is encoded as LSR #0
  if(immediate && amount == 0) {
    amount = 32;
  }

  const int adj_amount = std::min<int>(amount, 33);
  const u32 result = (u32)((u64)operand >> adj_amount);
  if(adj_amount != 0) {
    carry = ((u64)operand >> (adj_amount - 1)) & 1u;
  }
  operand = result;
}

void ASR(u32& operand, u8 amount, int& carry, bool immediate) {
  // ASR #32 is encoded as ASR #0
  if(immediate && amount == 0) {
    amount = 32;
  }

  const int adj_amount = std::min<int>(amount, 33);
  const u32 result = (u32)((s64)(s32)operand >> adj_amount);
  if(adj_amount != 0) {
    carry = ((s64)(s32)operand >> (adj_amount - 1)) & 1u;
  }
  operand = result;
}

void ROR(u32& operand, u8 amount, int& carry, bool immediate) {
  // RRX #1 is encoded as ROR #0
  if(immediate && amount == 0) {
    const int lsb = operand & 1;
    operand = (operand >> 1) | (carry << 31);
    carry = lsb;
  } else {
    if(amount == 0) {
      return;
    }
    const int adj_amount = amount & 31;
    operand = (operand >> adj_amount) | (operand << ((32 - adj_amount) & 31));
    carry = operand >> 31;
  }
}
