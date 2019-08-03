/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#pragma once

#include "../common/arm.hpp"
#include "../common/interface.hpp"

#include <array>
#include <utility>

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

  RegisterFile state;
  
private:
  
  /* Interface to emulator (Memory, SWI-emulation, ...). */
  Interface* interface;

  static auto GetRegisterBankByMode(Mode mode) -> Bank;
  void SwitchMode(Mode new_mode);
  void ReloadPipeline16();
  void ReloadPipeline32();
  void BuildConditionTable();
  bool CheckCondition(Condition condition);
  
  typedef void (ARM7TDMI::*Instruction16)(std::uint16_t);
  typedef void (ARM7TDMI::*Instruction32)(std::uint32_t);
  
  using OpcodeTable16 = std::array<Instruction16, 1024>;
  using OpcodeTable32 = std::array<Instruction32, 4096>;
  
  static OpcodeTable16 s_opcode_lut_thumb;
  static OpcodeTable32 s_opcode_lut_arm;
  
  #define ARM_INCLUDE_GUARD
  
  #include "memory.inl"
  #include "../common/arithmetic.inl"
  #include "isa-arm.inl"
  #include "isa-thumb.inl"
  #include "isa-emit.inl"
  
  #undef ARM_INCLUDE_GUARD

  StatusRegister* p_spsr;
  
  struct Pipeline {
    AccessType    fetch_type;
    std::uint32_t opcode[2];
  } pipe;
  
  bool condition_table[16][16];
};

#include "arm7tdmi.inl"

}