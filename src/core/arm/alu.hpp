
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

inline auto ARM::opADD(u32 op1, u32 op2, u32 op3, bool set_flags) -> u32 {
    if (set_flags) {
        u64 result64 = (u64)op1 + (u64)op2 + (u64)op3;
        u32 result32 = (u32)result64;

        updateZeroFlag(result32);
        updateSignFlag(result32);
        updateOverflowFlagAdd(result32, op1, op2); // how about op3?
        updateCarryFlag(result64 & 0x100000000ULL);

        //u32 overflow = ((~( op1        ^ op2) & ((op1 + op2)       ^ op2))  ^
        //                (~((op1 + op2) ^ op3) & ((op1 + op2 + op3) ^ op3))) >> 31;

        //if (overflow) {
        //    ctx.cpsr |=  MASK_VFLAG;
        //} else {
        //    ctx.cpsr &= ~MASK_VFLAG;
        //}

        return result32;
    }
    return op1 + op2 + op3;
}

inline auto ARM::opSUB(u32 op1, u32 op2, bool set_flags) -> u32 {
    if (set_flags) {
        u32 result = op1 - op2;

        updateZeroFlag(result);
        updateSignFlag(result);
        updateOverflowFlagSub(result, op1, op2);
        updateCarryFlag(op1 >= op2);

        return result;
    }
    return op1 - op2;
}

inline auto ARM::opSBC(u32 op1, u32 op2, u32 carry, bool set_flags) -> u32 {
    if (set_flags) {
        u32 result_1 = op1 - op2;
        u32 result_2 = result_1 - carry;;

        u32 overflow = (((op1 ^ op2) & ~(result_1 ^ op2)) ^
                       (result_1     & ~ result_2      )) >> 31;

        updateZeroFlag(result_2);
        updateSignFlag(result_2);
        updateCarryFlag((op1 >= op2) && (result_1 >= carry));

        if (overflow) {
            ctx.cpsr |=  MASK_VFLAG;
        } else {
            ctx.cpsr &= ~MASK_VFLAG;
        }

        return result_2;
    }
    return op1 - op2 - carry;
}
