/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <array>
#include <memory>
#include <utility>
#include "interface.hpp"
#include "state.hpp"
#include "util.hpp"

namespace ARM {

class ARM7 {
public:
    void Reset();

    auto GetInterface() -> Interface* const { return interface; }
    void SetInterface(Interface* interface) { this->interface = interface; }
    auto GetState() -> State& { return state; }

    void Run();
    void SignalIrq();

private:
    /* CPU registers */
    State state;
    StatusRegister* p_spsr;
    std::uint32_t pipe[2];

    /* Interface to emulator (Memory, SWI-emulation, ...). */
    Interface* interface;

    static Bank ModeToBank(Mode mode);
    void SwitchMode(Mode new_mode);

    bool CheckCondition(Condition condition);

    std::uint32_t ReadByte(std::uint32_t address, AccessType type);
    std::uint32_t ReadHalf(std::uint32_t address, AccessType type);
    std::uint32_t ReadWord(std::uint32_t address, AccessType type);
    std::uint32_t ReadByteSigned(std::uint32_t address, AccessType type);
    std::uint32_t ReadHalfRotate(std::uint32_t address, AccessType type);
    std::uint32_t ReadHalfSigned(std::uint32_t address, AccessType type);
    std::uint32_t ReadWordRotate(std::uint32_t address, AccessType type);
    void WriteByte(std::uint32_t address, std::uint8_t  value, AccessType type);
    void WriteHalf(std::uint32_t address, std::uint16_t value, AccessType type);
    void WriteWord(std::uint32_t address, std::uint32_t value, AccessType type);

    void SetNZ(std::uint32_t value);

    void LSL(std::uint32_t& operand, std::uint32_t amount, int& carry);
    void LSR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate);
    void ASR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate);
    void ROR(std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate);
    void DoShift(int opcode, std::uint32_t& operand, std::uint32_t amount, int& carry, bool immediate);

    std::uint32_t ADD(std::uint32_t op1, std::uint32_t op2, bool set_flags);
    std::uint32_t ADC(std::uint32_t op1, std::uint32_t op2, bool set_flags);
    std::uint32_t SUB(std::uint32_t op1, std::uint32_t op2, bool set_flags);
    std::uint32_t SBC(std::uint32_t op1, std::uint32_t op2, bool set_flags);

    void ARM_ReloadPipeline();
    void Thumb_ReloadPipeline();

    typedef void (ARM7::*ArmInstruction)(std::uint32_t);
    typedef void (ARM7::*ThumbInstruction)(std::uint16_t);

    /* Lookup-Tables */
    static std::array<ArmInstruction, 4096> arm_lut;
    static std::array<ThumbInstruction, 1024> thumb_lut;

    template <std::uint32_t instruction>
    static constexpr ARM7::ArmInstruction GetArmHandler();
    static constexpr std::array<ARM7::ArmInstruction, 4096> MakeArmLut();

    template <std::uint16_t instruction>
    static constexpr ARM7::ThumbInstruction GetThumbHandler();
    static constexpr std::array<ARM7::ThumbInstruction, 1024> MakeThumbLut();

    #include "isa/arm.inl"
    #include "isa/thumb.inl"
};

#include "arm.inl"
#include "memory.inl"
#include "alu/add_sub.inl"
#include "alu/shift.inl"

}