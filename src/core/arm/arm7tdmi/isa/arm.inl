/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

template <bool immediate, int opcode, bool _set_flags, int field4>
void ARM_DataProcessing(std::uint32_t instruction);

template <bool immediate, bool use_spsr, bool to_status>
void ARM_StatusTransfer(std::uint32_t instruction);

template <bool accumulate, bool set_flags>
void ARM_Multiply(std::uint32_t instruction);

template <bool sign_extend, bool accumulate, bool set_flags>
void ARM_MultiplyLong(std::uint32_t instruction);

template <bool byte>
void ARM_SingleDataSwap(std::uint32_t instruction);

void ARM_BranchAndExchange(std::uint32_t instruction);

template <bool pre, bool add, bool immediate, bool writeback, bool load, int opcode>
void ARM_HalfwordSignedTransfer(std::uint32_t instruction);

template <bool link>
void ARM_BranchAndLink(std::uint32_t instruction);

template <bool immediate, bool pre, bool add, bool byte, bool writeback, bool load>
void ARM_SingleDataTransfer(std::uint32_t instruction);

template <bool _pre, bool add, bool user_mode, bool _writeback, bool load>
void ARM_BlockDataTransfer(std::uint32_t instruction);

void ARM_Undefined(std::uint32_t instruction);

void ARM_SWI(std::uint32_t instruction);
