/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../arm7tdmi.hpp"

#include <util/meta.hpp>

using namespace ARM;

ARM7TDMI::OpcodeTable16 ARM7TDMI::s_opcode_lut_thumb = EmitAll16();
ARM7TDMI::OpcodeTable32 ARM7TDMI::s_opcode_lut_arm = EmitAll32();

constexpr ARM7TDMI::OpcodeTable16 ARM7TDMI::EmitAll16() {
    std::array<ARM7TDMI::Instruction16, 1024> lut = {};
    
    static_for<std::size_t, 0, 1024>([&](auto i) {
        lut[i] = EmitHandler16<i<<6>();
    });
    return lut;
}

constexpr ARM7TDMI::OpcodeTable32 ARM7TDMI::EmitAll32() {
    std::array<ARM7TDMI::Instruction32, 4096> lut = {};

    static_for<std::size_t, 0, 4096>([&](auto i) {
        lut[i] = EmitHandler32<((i & 0xFF0) << 16) | ((i & 0xF) << 4)>();
    });
    return lut;
}

template <std::uint16_t instruction>
constexpr ARM7TDMI::Instruction16 ARM7TDMI::EmitHandler16() {
    if ((instruction & 0xF800) < 0x1800) {
        // THUMB.1 Move shifted register
        return &ARM7TDMI::Thumb_MoveShiftedRegister<(instruction >> 11) & 3,     /* Operation */
                               (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF800) == 0x1800) {
        // THUMB.2 Add/subtract
        return &ARM7TDMI::Thumb_AddSub<(instruction >> 10) & 1,  /* Immediate-Flag */
                               (instruction >>  9) & 1,  /* Subtract-Flag */
                               (instruction >>  6) & 7>; /* 3-bit field */
    }
    if ((instruction & 0xE000) == 0x2000) {
        // THUMB.3 Move/compare/add/subtract immediate
        return &ARM7TDMI::Thumb_Op3<(instruction >> 11) & 3,   /* Operation */
                               (instruction >>  8) & 7>;  /* rD */
    }
    if ((instruction & 0xFC00) == 0x4000) {
        // THUMB.4 ALU operations
        return &ARM7TDMI::Thumb_ALU<(instruction >> 6) & 0xF>; /* Operation */
    }
    if ((instruction & 0xFC00) == 0x4400) {
        // THUMB.5 Hi register operations/branch exchange
        return &ARM7TDMI::Thumb_HighRegisterOps_BX<(instruction >> 8) & 3,  /* Operation */
                               (instruction >> 7) & 1,  /* High 1 */
                               (instruction >> 6) & 1>; /* High 2 */
    }
    if ((instruction & 0xF800) == 0x4800) {
        // THUMB.6 PC-relative load
        return &ARM7TDMI::Thumb_LoadStoreRelativePC<(instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xF200) == 0x5000) {
        // THUMB.7 Load/store with register offset
        return &ARM7TDMI::Thumb_LoadStoreOffsetReg<(instruction >> 10) & 3,  /* Operation (LB) */
                               (instruction >>  6) & 7>; /* rO */
    }
    if ((instruction & 0xF200) == 0x5200) {
        // THUMB.8 Load/store sign-extended byte/halfword
        return &ARM7TDMI::Thumb_LoadStoreSigned<(instruction >> 10) & 3,  /* Operation (HS) */
                               (instruction >>  6) & 7>; /* rO */
    }
    if ((instruction & 0xE000) == 0x6000) {
        // THUMB.9 Load store with immediate offset
        return &ARM7TDMI::Thumb_LoadStoreOffsetImm<(instruction >> 11) & 3,     /* Operation (BL) */
                               (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF000) == 0x8000) {
        // THUMB.10 Load/store halfword
        return &ARM7TDMI::Thumb_LoadStoreHword<(instruction >> 11) & 1,     /* Load-Flag */
                                (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF000) == 0x9000) {
        // THUMB.11 SP-relative load/store
        return &ARM7TDMI::Thumb_LoadStoreRelativeToSP<(instruction >> 11) & 1,  /* Load-Flag */
                                (instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xF000) == 0xA000) {
        // THUMB.12 Load address
        return &ARM7TDMI::Thumb_LoadAddress<(instruction >> 11) & 1,  /* 0=PC??? 1=SP */
                                (instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xFF00) == 0xB000) {
        // THUMB.13 Add offset to stack pointer
        return &ARM7TDMI::Thumb_AddOffsetToSP<(instruction >> 7) & 1>; /* Subtract-Flag */
    }
    if ((instruction & 0xF600) == 0xB400) {
        // THUMB.14 push/pop registers
        return &ARM7TDMI::Thumb_PushPop<(instruction >> 11) & 1,  /* 0=PUSH 1=POP */
                                (instruction >>  8) & 1>; /* PC/LR-flag */
    }
    if ((instruction & 0xF000) == 0xC000) {
        // THUMB.15 Multiple load/store
        return &ARM7TDMI::Thumb_LoadStoreMultiple<(instruction >> 11) & 1,  /* Load-Flag */
                                (instruction >>  8) & 7>; /* rB */
    }
    if ((instruction & 0xFF00) < 0xDF00) {
        // THUMB.16 Conditional Branch
        return &ARM7TDMI::Thumb_ConditionalBranch<(instruction >> 8) & 0xF>; /* Condition */
    }
    if ((instruction & 0xFF00) == 0xDF00) {
        // THUMB.17 Software Interrupt
        return &ARM7TDMI::Thumb_SWI;
    }
    if ((instruction & 0xF800) == 0xE000) {
        // THUMB.18 Unconditional Branch
        return &ARM7TDMI::Thumb_UnconditionalBranch;
    }
    if ((instruction & 0xF000) == 0xF000) {
        // THUMB.19 Long branch with link
        return &ARM7TDMI::Thumb_LongBranchLink<(instruction >> 11) & 1>; /* Second Instruction */
    }

    return &ARM7TDMI::Thumb_Undefined;
}

template <std::uint32_t instruction>
constexpr ARM7TDMI::Instruction32 ARM7TDMI::EmitHandler32() {
    const std::uint32_t opcode = instruction & 0x0FFFFFFF;
    
    const bool pre  = instruction & (1 << 24);
    const bool add  = instruction & (1 << 23);
    const bool wb   = instruction & (1 << 21);
    const bool load = instruction & (1 << 20);

    switch (opcode >> 26) {
    case 0b00:
        if (opcode & (1 << 25)) {
            // ARM.8 Data processing and PSR transfer ... immediate
            const bool set_flags = instruction & (1 << 20);
            const int  opcode = (instruction >> 21) & 0xF;

            if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
                const bool use_spsr  = instruction & (1 << 22);
                const bool to_status = instruction & (1 << 21);

                return &ARM7TDMI::ARM_StatusTransfer<true, use_spsr, to_status>;
            } else {
                const int field4 = (instruction >> 4) & 0xF;

                return &ARM7TDMI::ARM_DataProcessing<true, opcode, set_flags, field4>;
            }
        } else if ((opcode & 0xFF000F0) == 0x1200010) {
            // ARM.3 Branch and exchange
            // TODO: Some bad instructions might be falsely detected as BX.
            // How does HW handle this?
            return &ARM7TDMI::ARM_BranchAndExchange;
        } else if ((opcode & 0x10000F0) == 0x0000090) {
            // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
            const bool accumulate = instruction & (1 << 21);
            const bool set_flags  = instruction & (1 << 20);

            if (opcode & (1 << 23)) {
                const bool sign_extend = instruction & (1 << 22);

                return &ARM7TDMI::ARM_MultiplyLong<sign_extend, accumulate, set_flags>;
            } else {
                return &ARM7TDMI::ARM_Multiply<accumulate, set_flags>;
            }
        } else if ((opcode & 0x10000F0) == 0x1000090) {
            // ARM.4 Single data swap
            const bool byte = instruction & (1 << 22);

            return &ARM7TDMI::ARM_SingleDataSwap<byte>;
        } else if ((opcode & 0xF0) == 0xB0 ||
                   (opcode & 0xD0) == 0xD0) {
            // ARM.5 Halfword data transfer, register offset
            // ARM.6 Halfword data transfer, immediate offset
            // ARM.7 Signed data transfer (byte/halfword)
            const bool immediate = instruction & (1 << 22);
            const int opcode = (instruction >> 5) & 3;

            return &ARM7TDMI::ARM_HalfwordSignedTransfer<pre, add, immediate, wb, load, opcode>;
        } else {
            // ARM.8 Data processing and PSR transfer
            const bool set_flags = instruction & (1 << 20);
            const int  opcode = (instruction >> 21) & 0xF;

            if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
                const bool use_spsr  = instruction & (1 << 22);
                const bool to_status = instruction & (1 << 21);

                return &ARM7TDMI::ARM_StatusTransfer<false, use_spsr, to_status>;
            } else {
                const int field4 = (instruction >> 4) & 0xF;

                return &ARM7TDMI::ARM_DataProcessing<false, opcode, set_flags, field4>;
            }
        }
        break;
    case 0b01:
        // ARM.9 Single data transfer, ARM.10 Undefined
        if ((opcode & 0x2000010) == 0x2000010) {
            return &ARM7TDMI::ARM_Undefined;
        } else {
            const bool immediate = ~instruction & (1 << 25);
            const bool byte = instruction & (1 << 22);

            return &ARM7TDMI::ARM_SingleDataTransfer<immediate, pre, add, byte, wb, load>;
        }
        break;
    case 0b10:
        // ARM.11 Block data transfer, ARM.12 Branch
        if (opcode & (1 << 25)) {
            return &ARM7TDMI::ARM_BranchAndLink<(opcode >> 24) & 1>;
        } else {
            const bool user_mode = instruction & (1 << 22);

            return &ARM7TDMI::ARM_BlockDataTransfer<pre, add, user_mode, wb, load>;
        }
        break;
    case 0b11:
        if (opcode & (1 << 25)) {
            if (opcode & (1 << 24)) {
                // ARM.16 Software interrupt
                return &ARM7TDMI::ARM_SWI;
            } else {
                // ARM.14 Coprocessor data operation
                // ARM.15 Coprocessor register transfer
            }
        } else {
            // ARM.13 Coprocessor data transfer
        }
        break;
    }

    return &ARM7TDMI::ARM_Undefined;
}
