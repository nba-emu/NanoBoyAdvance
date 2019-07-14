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

    typedef void (ARM7TDMI::*Instruction16)(std::uint16_t);
    typedef void (ARM7TDMI::*Instruction32)(std::uint32_t);
    
    void ReloadPipeline16();
    void ReloadPipeline32();

    using OpcodeTable16 = std::array<Instruction16, 1024>;
    using OpcodeTable32 = std::array<Instruction32, 4096>;
    
    static OpcodeTable16 s_opcode_lut_thumb;
    static OpcodeTable32 s_opcode_lut_arm;

    template <std::uint16_t instruction>
    static constexpr ARM7TDMI::Instruction16 EmitHandler16();
    static constexpr OpcodeTable16 EmitAll16();
    
    template <std::uint32_t instruction>
    static constexpr ARM7TDMI::Instruction32 EmitHandler32();
    static constexpr OpcodeTable32 EmitAll32();

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