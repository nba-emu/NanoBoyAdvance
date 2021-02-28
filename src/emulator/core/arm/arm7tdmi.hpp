/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <common/likely.hpp>
#include <common/log.hpp>
#include <emulator/core/scheduler.hpp>

#include "memory.hpp"
#include "state.hpp"

namespace nba::core::arm {

class ARM7TDMI {
public:
  using Access = MemoryBase::Access;

  ARM7TDMI(Scheduler& scheduler, MemoryBase* interface)
      : scheduler(scheduler)
      , interface(interface) {
    Reset();
  }

  auto IRQLine() -> bool& { return irq_line; }

  void Reset() {
    state.Reset();

    SwitchMode(MODE_SYS);

    pipe.opcode[0] = 0xF0000000;
    pipe.opcode[1] = 0xF0000000;
    pipe.fetch_type = Access::Nonsequential;
    irq_line = false;
    ldm_usermode_conflict = false;
    cpu_mode_is_invalid = false;
  }

  auto GetPrefetchedOpcode(int slot) -> std::uint32_t {
    return pipe.opcode[slot];
  }

  void Run() {
    if (IRQLine()) SignalIRQ();

    auto instruction = pipe.opcode[0];

    if (state.cpsr.f.thumb) {
      state.r15 &= ~1;

      pipe.opcode[0] = pipe.opcode[1];
      pipe.opcode[1] = ReadHalf(state.r15, pipe.fetch_type);
      (this->*s_opcode_lut_16[instruction >> 6])(instruction);
    } else {
      state.r15 &= ~3;

      pipe.opcode[0] = pipe.opcode[1];
      pipe.opcode[1] = ReadWord(state.r15, pipe.fetch_type);
      if (CheckCondition(static_cast<Condition>(instruction >> 28))) {
        int hash = ((instruction >> 16) & 0xFF0) |
                   ((instruction >>  4) & 0x00F);
        (this->*s_opcode_lut_32[hash])(instruction);
      } else {
        pipe.fetch_type = Access::Sequential;
        state.r15 += 4;
      }
    }
  }

  RegisterFile state;

  typedef void (ARM7TDMI::*Handler16)(std::uint16_t);
  typedef void (ARM7TDMI::*Handler32)(std::uint32_t);

private:
  friend struct TableGen;

  auto GetReg(int id) -> std::uint32_t {
    std::uint32_t result = 0;
    bool is_banked = id >= 8 && id != 15;

    if (unlikely(ldm_usermode_conflict && is_banked)) {
      result |= state.bank[BANK_NONE][id - 8];
    }

    if (likely(!cpu_mode_is_invalid || !is_banked)) {
      result |= state.reg[id];
    }

    return result;
  }

  void SetReg(int id, std::uint32_t value) {
    bool is_banked = id >= 8 && id != 15;

    if (unlikely(ldm_usermode_conflict && is_banked)) {
      state.bank[BANK_NONE][id - 8] = value;
    }

    if (likely(!cpu_mode_is_invalid || !is_banked)) {
      state.reg[id] = value;
    }
  }

  auto GetSPSR() -> StatusRegister {
    std::uint32_t spsr = 0;

    if (unlikely(ldm_usermode_conflict)) {
      /* TODO: current theory is that the value gets OR'd with CPSR,
       * because in user and system mode SPSR reads return the CPSR value.
       * But this needs to be confirmed.
       */
      spsr |= state.cpsr.v;
    }

    if (unlikely(cpu_mode_is_invalid)) {
      // TODO: where does this value come from?
      spsr |= 0x00000010;
    } else {
      spsr |= p_spsr->v;
    }

    return StatusRegister{spsr};
  }

  void SignalIRQ() {
    if (state.cpsr.f.mask_irq) {
      return;
    }

    // Prefetch the next instruction
    // The result will be discarded because we flush the pipeline.
    // But this is important for timing nonetheless.
    if (state.cpsr.f.thumb) {
      ReadHalf(state.r15 & ~1, pipe.fetch_type);
    } else {
      ReadWord(state.r15 & ~3, pipe.fetch_type);
    }

    // Save current program status register.
    state.spsr[BANK_IRQ].v = state.cpsr.v;

    // Enter IRQ mode and disable IRQs.
    SwitchMode(MODE_IRQ);
    state.cpsr.f.mask_irq = 1;

    // Save current program counter and disable Thumb.
    if (state.cpsr.f.thumb) {
      state.cpsr.f.thumb = 0;
      SetReg(14, state.r15);
    } else {
      SetReg(14, state.r15 - 4);
    }

    // Jump to IRQ exception vector.
    state.r15 = 0x18;
    ReloadPipeline32();
  }

  bool CheckCondition(Condition condition) {
    if (condition == COND_AL)
      return true;
    return s_condition_lut[(static_cast<int>(condition) << 4) | (state.cpsr.v >> 28)];
  }

  void ReloadPipeline16() {
    pipe.opcode[0] = interface->ReadHalf(state.r15 + 0, Access::Nonsequential);
    pipe.opcode[1] = interface->ReadHalf(state.r15 + 2, Access::Sequential);
    pipe.fetch_type = Access::Sequential;
    state.r15 += 4;
  }

  void ReloadPipeline32() {
    pipe.opcode[0] = interface->ReadWord(state.r15 + 0, Access::Nonsequential);
    pipe.opcode[1] = interface->ReadWord(state.r15 + 4, Access::Sequential);
    pipe.fetch_type = Access::Sequential;
    state.r15 += 8;
  }

  auto GetRegisterBankByMode(Mode mode) -> Bank {
    switch (mode) {
    case MODE_USR:
    case MODE_SYS:
      return BANK_NONE;
    case MODE_FIQ:
      return BANK_FIQ;
    case MODE_IRQ:
      return BANK_IRQ;
    case MODE_SVC:
      return BANK_SVC;
    case MODE_ABT:
      return BANK_ABT;
    case MODE_UND:
      return BANK_UND;
    }

    return BANK_INVALID;
  }

  void SwitchMode(Mode new_mode) {
    auto old_bank = GetRegisterBankByMode(state.cpsr.f.mode);
    auto new_bank = GetRegisterBankByMode(new_mode);

    state.cpsr.f.mode = new_mode;

    if (new_bank != BANK_NONE) {
      p_spsr = &state.spsr[new_bank];
    } else {
      /* In system/user mode reading from SPSR returns the current CPSR value.
       * However writes to SPSR appear to do nothing.
       * We take care of this fact in the MSR implementation.
       */
      p_spsr = &state.cpsr;
    }

    if (old_bank == new_bank) {
      return;
    }

    if (old_bank == BANK_FIQ || new_bank == BANK_FIQ) {
      for (int i = 0; i < 7; i++) {
        state.bank[old_bank][i] = state.reg[8 + i];
      }

      for (int i = 0; i < 7; i++) {
        state.reg[8 + i] = state.bank[new_bank][i];
      }
    } else {
      state.bank[old_bank][5] = state.r13;
      state.bank[old_bank][6] = state.r14;

      state.r13 = state.bank[new_bank][5];
      state.r14 = state.bank[new_bank][6];
    }

    cpu_mode_is_invalid = new_bank == BANK_INVALID;
  }

  #include "handlers/arithmetic.inl"
  #include "handlers/handler16.inl"
  #include "handlers/handler32.inl"
  #include "handlers/memory.inl"

  Scheduler& scheduler;
  MemoryBase* interface;
  StatusRegister* p_spsr;
  bool ldm_usermode_conflict;
  bool cpu_mode_is_invalid;

  struct Pipeline {
    Access fetch_type;
    std::uint32_t opcode[2];
  } pipe;

  bool irq_line;

  static std::array<bool, 256> s_condition_lut;
  static std::array<Handler16, 1024> s_opcode_lut_16;
  static std::array<Handler32, 4096> s_opcode_lut_32;
};

} // namespace nba::core::arm
