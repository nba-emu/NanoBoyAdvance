/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../core/arm.hpp"
#include "../core/interface.hpp"

#include <array>
#include <memory>
#include <utility>
#include <util/meta.hpp>

namespace ARM {

class ARM7TDMI {

public:
    ARM7TDMI(Interface* interface)
        : interface(interface)
    {
        BuildConditionTable();
        Reset();
    }

    auto GetInterface() -> Interface* const { return interface; }

    void Reset();
    void Run();
    void SignalIrq();

    State state;
    
private:
    
    
    /* Interface to emulator (Memory, SWI-emulation, ...). */
    Interface* interface;

    void SwitchMode(Mode new_mode);

    void BuildConditionTable();
    bool CheckCondition(Condition condition);
    
    #define ARM_INCLUDE_GUARD
    
    #include "memory.inl"
    #include "../core/arithmetic.inl"
    
    #undef ARM_INCLUDE_GUARD

    void ARM_ReloadPipeline();
    void Thumb_ReloadPipeline();

    typedef void (ARM7TDMI::*ArmInstruction)(std::uint32_t);
    typedef void (ARM7TDMI::*ThumbInstruction)(std::uint16_t);
    
    static std::array<ArmInstruction, 4096> arm_lut;
    static std::array<ThumbInstruction, 1024> thumb_lut;

    template <std::uint32_t instruction>
    static constexpr ARM7TDMI::ArmInstruction GetArmHandler();
    
    static constexpr std::array<ARM7TDMI::ArmInstruction, 4096> MakeArmLut();

    template <std::uint16_t instruction>
    static constexpr ARM7TDMI::ThumbInstruction GetThumbHandler();
    
    static constexpr std::array<ARM7TDMI::ThumbInstruction, 1024> MakeThumbLut();

    StatusRegister* p_spsr;
    
    struct Pipeline {
        AccessType    fetch_type;
        std::uint32_t opcode[2];
    } pipe;
    
    bool condition_table[16][16];
    
    #include "isa/arm.inl"
    #include "isa/thumb.inl"
};

#include "arm7tdmi.inl"

}