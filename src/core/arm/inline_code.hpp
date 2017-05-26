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

#define Z_FLAG (ctx.cpsr & MASK_ZFLAG)
#define C_FLAG (ctx.cpsr & MASK_CFLAG)
#define N_FLAG (ctx.cpsr & MASK_NFLAG)
#define V_FLAG (ctx.cpsr & MASK_VFLAG)

#define XOR_BIT_31(a, b) (((a) ^ (b)) >> 31)

// MEH: place this somewhere else. It should be inline though :/
inline void ARM::step() {
  auto& pipe = ctx.pipe;

  if (ctx.cpsr & MASK_THUMB) {
    ctx.r15 &= ~1;

    if (pipe.index == 0) {
      pipe.opcode[2] = bus_read_hword(ctx.r15, MEM_NONE);
    } else {
      pipe.opcode[pipe.index - 1] = bus_read_hword(ctx.r15, MEM_NONE);
    }

    thumb_execute(pipe.opcode[pipe.index]);
    
    if (pipe.do_flush) {
      refill_pipeline();
      return;
    }
    
    if (++pipe.index == 3) pipe.index = 0;
    
    ctx.r15 += 2;
  } else {
    ctx.r15 &= ~3;

    if (pipe.index == 0) {
      pipe.opcode[2] = bus_read_word(ctx.r15, MEM_NONE);
    } else {
      pipe.opcode[pipe.index - 1] = bus_read_word(ctx.r15, MEM_NONE);
    }

    arm_execute(pipe.opcode[pipe.index]);
    
    if (pipe.do_flush) {
      refill_pipeline();
      return;
    }
    
    if (++pipe.index == 3) pipe.index = 0;
    
    ctx.r15 += 4;
  }
}

inline bool ARM::check_condition(Condition condition) {
  if (condition == COND_AL) {
    return true;
  }

  switch (condition) {
    case COND_EQ: return Z_FLAG;
    case COND_NE: return !Z_FLAG;
    case COND_CS: return C_FLAG;
    case COND_CC: return !C_FLAG;
    case COND_MI: return N_FLAG;
    case COND_PL: return !N_FLAG;
    case COND_VS: return V_FLAG;
    case COND_VC: return !V_FLAG;
    case COND_HI: return C_FLAG && !Z_FLAG;
    case COND_LS: return !C_FLAG || Z_FLAG;
    case COND_GE: return N_FLAG == V_FLAG;
    case COND_LT: return N_FLAG != V_FLAG;
    case COND_GT: return !Z_FLAG && (N_FLAG == V_FLAG);
    case COND_LE: return Z_FLAG || (N_FLAG != V_FLAG);
    case COND_AL: return true;
    case COND_NV: return false;
  }
  
  return false;
}

inline void ARM::update_sign_flag(u32 result) {
  if (result >> 31) {
    ctx.cpsr |= MASK_NFLAG;
  } else {
    ctx.cpsr &= ~MASK_NFLAG;
  }
}

inline void ARM::update_zero_flag(u64 result) {
  if (result == 0) {
    ctx.cpsr |= MASK_ZFLAG;
  } else {
    ctx.cpsr &= ~MASK_ZFLAG;
  }
}

inline void ARM::update_carry_flag(bool carry) {
  if (carry) {
    ctx.cpsr |= MASK_CFLAG;
  } else {
    ctx.cpsr &= ~MASK_CFLAG;
  }
}

inline void ARM::update_overflow_add(u32 result, u32 operand1, u32 operand2) {
  bool overflow = !XOR_BIT_31(operand1, operand2) && XOR_BIT_31(result, operand2);

  if (overflow) {
    ctx.cpsr |= MASK_VFLAG;
  } else {
    ctx.cpsr &= ~MASK_VFLAG;
  }
}

inline void ARM::update_overflow_sub(u32 result, u32 operand1, u32 operand2) {
  bool overflow = XOR_BIT_31(operand1, operand2) && !XOR_BIT_31(result, operand2);

  if (overflow) {
    ctx.cpsr |= MASK_VFLAG;
  } else {
    ctx.cpsr &= ~MASK_VFLAG;
  }
}

inline void ARM::shift_lsl(u32& operand, u32 amount, bool& carry) {
  if (amount == 0) {
    return;
  }

  for (u32 i = 0; i < amount; i++) {
    carry = operand & 0x80000000 ? true : false;
    operand <<= 1;
  }
}

inline void ARM::shift_lsr(u32& operand, u32 amount, bool& carry, bool immediate) {
  // LSR #0 equals to LSR #32
  amount = immediate & (amount == 0) ? 32 : amount;
  
  for (u32 i = 0; i < amount; i++) {
    carry = operand & 1 ? true : false;
    operand >>= 1;
  }
}

inline void ARM::shift_asr(u32& operand, u32 amount, bool& carry, bool immediate) {
  u32 sign_bit = operand & 0x80000000;

  // ASR #0 equals to ASR #32
  amount = (immediate && (amount == 0)) ? 32 : amount;

  for (u32 i = 0; i < amount; i++) {
    carry   = operand & 1 ? true : false;
    operand = (operand >> 1) | sign_bit;
  }
}

inline void ARM::shift_ror(u32& operand, u32 amount, bool& carry, bool immediate) {
  // ROR #0 equals to RRX #1
  if (amount != 0 || !immediate) {
    for (u32 i = 1; i <= amount; i++) {
      u32 high_bit = (operand & 1) ? 0x80000000 : 0;
      
      operand = (operand >> 1) | high_bit;
      carry   = high_bit == 0x80000000;
    }
  } else {
    bool old_carry = carry;
    
    carry   = (operand & 1) ? true : false;
    operand = (operand >> 1) | (old_carry ? 0x80000000 : 0);
  }
}

inline void ARM::apply_shift(int shift, u32& operand, u32 amount, bool& carry, bool immediate) {
  switch (shift) {
  case 0:
    shift_lsl(operand, amount, carry);
    return;
  case 1:
    shift_lsr(operand, amount, carry, immediate);
    return;
  case 2:
    shift_asr(operand, amount, carry, immediate);
    return;
  case 3:
    shift_ror(operand, amount, carry, immediate);
  }
}

inline u32 ARM::read_byte(u32 address, int flags) {
  u32 value = bus_read_byte(address, flags);
  
  if ((flags & MEM_SIGNED) && (value & 0x80)) {
    return value | 0xFFFFFF00;
  }
  
  return value;
}

inline u32 ARM::read_hword(u32 address, int flags) {  
  u32 value;
  
  if (flags & MEM_ROTATE) {
    if (address & 1) {
      value = bus_read_hword(address & ~1, flags);
      return (value >> 8) | (value << 24);
    }
    return bus_read_hword(address, flags);
  }
  
  if (flags & MEM_SIGNED) {
    if (address & 1) {
      value = bus_read_byte(address, flags);
      if (value & 0x80) {
        return value | 0xFFFFFF00;
      }
      return value;
    } else {
      value = bus_read_hword(address, flags);
      if (value & 0x8000) {
        return value | 0xFFFF0000;
      }
      return value;
    }
  }
  
  return bus_read_hword(address & ~1, flags);
}

inline u32 ARM::read_word(u32 address, int flags) {
  u32 value = bus_read_word(address & ~3, flags);
  
  if (flags & MEM_ROTATE) {
    int amount = (address & 3) << 3;
    return (value >> amount) | (value << (32 - amount));
  }
  
  return value;
}

inline void ARM::write_byte(u32 address, u8 value, int flags) {
  bus_write_byte(address, value, flags);
}

inline void ARM::write_hword(u32 address, u16 value, int flags) {
  bus_write_hword(address & ~1, value, flags);
}

inline void ARM::write_word(u32 address, u32 value, int flags) {
  bus_write_word(address & ~3, value, flags);
}

inline void ARM::refill_pipeline() {
  if (ctx.cpsr & MASK_THUMB) {
    ctx.pipe.opcode[0] = bus_read_hword(ctx.r15, MEM_NONE);
    ctx.pipe.opcode[1] = bus_read_hword(ctx.r15 + 2, MEM_NONE);
    ctx.r15 += 4;
  } else {
    ctx.pipe.opcode[0] = bus_read_word(ctx.r15, MEM_NONE);
    ctx.pipe.opcode[1] = bus_read_word(ctx.r15 + 4, MEM_NONE);
    ctx.r15 += 8;
  }
  
  ctx.pipe.index = 0;
  ctx.pipe.do_flush = false;
}

