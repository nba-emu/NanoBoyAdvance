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

#ifdef ARMIGO_INCLUDE

#define Z_FLAG (m_cpsr & MASK_ZFLAG)
#define C_FLAG (m_cpsr & MASK_CFLAG)
#define N_FLAG (m_cpsr & MASK_NFLAG)
#define V_FLAG (m_cpsr & MASK_VFLAG)

#define XOR_BIT_31(a, b) (((a) ^ (b)) >> 31)

inline bool check_condition(Condition condition) {
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

inline void update_sign(u32 result) {
    if (result >> 31) {
        m_cpsr |= MASK_NFLAG;
    } else {
        m_cpsr &= ~MASK_NFLAG;
    }
}

inline void update_zero(u64 result) {
    if (result == 0) {
        m_cpsr |= MASK_ZFLAG;
    } else {
        m_cpsr &= ~MASK_ZFLAG;
    }
}

inline void set_carry(bool carry) {
    if (carry) {
        m_cpsr |= MASK_CFLAG;
    } else {
        m_cpsr &= ~MASK_CFLAG;
    }
}

inline void update_overflow_add(u32 result, u32 operand1, u32 operand2) {
    bool overflow = !XOR_BIT_31(operand1, operand2) && XOR_BIT_31(result, operand2);

    if (overflow) {
        m_cpsr |= MASK_VFLAG;
    } else {
        m_cpsr &= ~MASK_VFLAG;
    }
}

inline void update_overflow_sub(u32 result, u32 operand1, u32 operand2) {
    bool overflow = XOR_BIT_31(operand1, operand2) && !XOR_BIT_31(result, operand2);

    if (overflow) {
        m_cpsr |= MASK_VFLAG;
    } else {
        m_cpsr &= ~MASK_VFLAG;
    }
}

#endif
