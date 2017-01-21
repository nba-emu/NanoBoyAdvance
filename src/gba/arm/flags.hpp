///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#ifdef ARM_INCLUDE

inline bool check_condition(cpu_condition condition)
{
    if (condition == COND_AL) return true;

    switch (condition)
    {
    case COND_EQ: return m_cpsr & MASK_ZFLAG;
    case COND_NE: return ~m_cpsr & MASK_ZFLAG;
    case COND_CS: return m_cpsr & MASK_CFLAG;
    case COND_CC: return ~m_cpsr & MASK_CFLAG;
    case COND_MI: return m_cpsr & MASK_NFLAG;
    case COND_PL: return ~m_cpsr & MASK_NFLAG;
    case COND_VS: return m_cpsr & MASK_VFLAG;
    case COND_VC: return ~m_cpsr & MASK_VFLAG;
    case COND_HI: return (m_cpsr & MASK_CFLAG) && (~m_cpsr & MASK_ZFLAG);
    case COND_LS: return (~m_cpsr & MASK_CFLAG) || (m_cpsr & MASK_ZFLAG);
    case COND_GE: return (m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG);
    case COND_LT: return (m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG);
    case COND_GT: return (~m_cpsr & MASK_ZFLAG) && ((m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG));
    case COND_LE: return (m_cpsr & MASK_ZFLAG) || ((m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG));
    case COND_AL: return true; // dummy
    case COND_NV: return false;
    }
    return false;
}

inline void update_sign(u32 result)
{
    m_cpsr = result & 0x80000000 ? (m_cpsr | MASK_NFLAG) : (m_cpsr & ~MASK_NFLAG);
}

inline void update_zero(u64 result)
{
    m_cpsr = result == 0 ? (m_cpsr | MASK_ZFLAG) : (m_cpsr & ~MASK_ZFLAG);
}

inline void set_carry(bool carry)
{
    m_cpsr = carry ? (m_cpsr | MASK_CFLAG) : (m_cpsr & ~MASK_CFLAG);
}

inline void update_overflow_add(u32 result, u32 operand1, u32 operand2)
{
    bool overflow = !(((operand1) ^ (operand2)) >> 31) && ((result) ^ (operand2)) >> 31;
    m_cpsr = overflow ? (m_cpsr | MASK_VFLAG) : (m_cpsr & ~MASK_VFLAG);
}

inline void update_overflow_sub(u32 result, u32 operand1, u32 operand2)
{
    bool overflow = ((operand1) ^ (operand2)) >> 31 && !(((result) ^ (operand2)) >> 31);
    m_cpsr = overflow ? (m_cpsr | MASK_VFLAG) : (m_cpsr & ~MASK_VFLAG);
}

#endif
