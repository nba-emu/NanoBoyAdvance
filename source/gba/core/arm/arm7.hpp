/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>

#include "interface.hpp"
#include "state.hpp"

namespace ARM {

class ARM7TDMI {
  
public:
  ARM7TDMI(Interface* interface)
    : interface(interface)
  {
    BuildConditionTable();
    Reset();
  }

  void Reset();
  void Run();
  void SignalIRQ();
  
  auto GetPrefetchedOpcode(int slot) -> std::uint32_t {
    return pipe.opcode[slot];
  }
  
  RegisterFile state;

  typedef void (ARM7TDMI::*Handler16)(std::uint16_t);
  typedef void (ARM7TDMI::*Handler32)(std::uint32_t);
  
private:
  friend struct TableGen;
  
  /* Interface to emulator (Memory, SWI-emulation, ...). */
  Interface* interface;

  static auto GetRegisterBankByMode(Mode mode) -> Bank;
  void SwitchMode(Mode new_mode);
  void ReloadPipeline16();
  void ReloadPipeline32();
  void BuildConditionTable();
  bool CheckCondition(Condition condition);
 
  #define ARM_INCLUDE_GUARD
  
  #include "isa-armv4/arithmetic.inl"
  #include "isa-armv4/isa-arm.inl"
  #include "isa-armv4/isa-thumb.inl"
  #include "isa-armv4/memory.inl"
  
  #undef ARM_INCLUDE_GUARD

  StatusRegister* p_spsr;
  
  struct Pipeline {
    AccessType    fetch_type;
    std::uint32_t opcode[2];
  } pipe;
  
  bool condition_table[16][16];

  static std::array<Handler16, 1024> s_opcode_lut_16;
  static std::array<Handler32, 4096> s_opcode_lut_32;
};

#include "arm7.inl"

}