/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
void Thumb_MoveShiftedRegister(std::uint16_t instruction);

template <bool immediate, bool subtract, int field3>
void Thumb_AddSub(std::uint16_t instruction);

template <int op, int dst>
void Thumb_Op3(std::uint16_t instruction);

template <int op>
void Thumb_ALU(std::uint16_t instruction);

template <int op, bool high1, bool high2>
void Thumb_HighRegisterOps_BX(std::uint16_t instruction);

template <int dst>
void Thumb_LoadStoreRelativePC(std::uint16_t instruction);

template <int op, int off>
void Thumb_LoadStoreOffsetReg(std::uint16_t instruction);

template <int op, int off>
void Thumb_LoadStoreSigned(std::uint16_t instruction);

template <int op, int imm>
void Thumb_LoadStoreOffsetImm(std::uint16_t instruction);

template <bool load, int imm>
void Thumb_LoadStoreHword(std::uint16_t instruction);

template <bool load, int dst>
void Thumb_LoadStoreRelativeToSP(std::uint16_t instruction);

template <bool stackptr, int dst>
void Thumb_LoadAddress(std::uint16_t instruction);

template <bool sub>
void Thumb_AddOffsetToSP(std::uint16_t instruction);

template <bool pop, bool rbit>
void Thumb_PushPop(std::uint16_t instruction);

template <bool load, int base>
void Thumb_LoadStoreMultiple(std::uint16_t instruction);

template <int cond>
void Thumb_ConditionalBranch(std::uint16_t instruction);

void Thumb_SoftwareInterrupt(std::uint16_t instruction);

void Thumb_UnconditionalBranch(std::uint16_t instruction);

template <bool second_instruction>
void Thumb_LongBranchLink(std::uint16_t instruction);

void Thumb_Undefined(std::uint16_t instruction);
