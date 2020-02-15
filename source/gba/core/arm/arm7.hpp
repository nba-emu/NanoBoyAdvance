/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "interface.hpp"
#include "state.hpp"

#include <array>
#include <utility>
#include <type_traits>

namespace ARM {

template <typename Tinterface>
class ARM7TDMI {
  
public:
  ARM7TDMI(Tinterface* interface)
    : interface(interface)
  {
    static_assert(std::is_same<Interface, Tinterface>::value ||
                  std::is_base_of<Interface, Tinterface>::value,
                  "Tinterface must implement ARM::Interface");
    opcode_lut_16 = EmitAll16();
    opcode_lut_32 = EmitAll32();
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
  
private:
  
  /* Interface to emulator (Memory, SWI-emulation, ...). */
  Tinterface* interface;

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
  
  OpcodeTable16 opcode_lut_16;
  OpcodeTable32 opcode_lut_32;
  
  #define ARM_INCLUDE_GUARD
  
  #include "isa-armv4/arithmetic.inl"
  #include "isa-armv4/emit.inl"
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
};

#include "arm7.inl"

}