/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <nba/common/compiler.hpp>
#include <nba/log.hpp>
#include <nba/save_state.hpp>
#include <nba/scheduler.hpp>

#include "bus/bus.hpp"
#include "arm/state.hpp"

namespace nba::core::arm {

struct ARM7TDMI {
  using Access = Bus::Access;

  ARM7TDMI(Scheduler& scheduler, Bus& bus)
      : scheduler(scheduler)
      , bus(bus) {
    scheduler.Register(Scheduler::EventClass::ARM_ldm_usermode_conflict, this, &ARM7TDMI::ClearLDMUsermodeConflictFlag);

    Reset();
  }

  auto IRQLine() -> bool& { return irq_line; }

  void Reset() {
    state.Reset();
    SwitchMode(state.cpsr.f.mode);

    pipe.opcode[0] = 0xF0000000;
    pipe.opcode[1] = 0xF0000000;
    pipe.access = Access::Code | Access::Nonsequential;
    irq_line = false;
    latch_irq_disable = state.cpsr.f.mask_irq;
    ldm_usermode_conflict = false;
    cpu_mode_is_invalid = false;
  }

  auto GetFetchedOpcode(int slot) -> u32 {
    return pipe.opcode[slot];
  }

  void Run() {
    if(IRQLine()) SignalIRQ();

    auto instruction = pipe.opcode[0];

    latch_irq_disable = state.cpsr.f.mask_irq;

    state.r15 &= ~1;

    if(state.cpsr.f.thumb) {
      pipe.opcode[0] = pipe.opcode[1];
      pipe.opcode[1] = ReadHalf(state.r15, pipe.access);

      (this->*s_opcode_lut_16[instruction >> 6])(instruction);
    } else {
      pipe.opcode[0] = pipe.opcode[1];
      pipe.opcode[1] = ReadWord(state.r15, pipe.access);

      if(CheckCondition(static_cast<Condition>(instruction >> 28))) {
        int hash = ((instruction >> 16) & 0xFF0) |
                   ((instruction >>  4) & 0x00F);
        (this->*s_opcode_lut_32[hash])(instruction);
      } else {
        pipe.access = Access::Code | Access::Sequential;
        state.r15 += 4;
      }
    }
  }

  void SwitchMode(Mode new_mode) {
    auto old_bank = GetRegisterBankByMode(state.cpsr.f.mode);
    auto new_bank = GetRegisterBankByMode(new_mode);

    state.cpsr.f.mode = new_mode;

    if(new_bank != BANK_NONE) {
      p_spsr = &state.spsr[new_bank];
    } else {
      /* In system/user mode reading from SPSR returns the current CPSR value.
       * However writes to SPSR appear to do nothing.
       * We take care of this fact in the MSR implementation.
       */
      p_spsr = &state.cpsr;
    }

    if(old_bank == new_bank) {
      return;
    }

    if(old_bank == BANK_FIQ) {
      for(int i = 0; i < 5; i++) {
        state.bank[BANK_FIQ][i] = state.reg[8 + i];
      }

      for(int i = 0; i < 5; i++) {
        state.reg[8 + i] = state.bank[BANK_NONE][i];
      }
    } else if(new_bank == BANK_FIQ) {
      for(int i = 0; i < 5; i++) {
        state.bank[BANK_NONE][i] = state.reg[8 + i];
      }

      for(int i = 0; i < 5; i++) {
        state.reg[8 + i] = state.bank[new_bank][i];
      }
    }

    state.bank[old_bank][5] = state.r13;
    state.bank[old_bank][6] = state.r14;

    state.r13 = state.bank[new_bank][5];
    state.r14 = state.bank[new_bank][6];

    cpu_mode_is_invalid = new_bank == BANK_INVALID;
  }

  void LoadState(SaveState const& save_state);
  void CopyState(SaveState& save_state);

  RegisterFile state;

  typedef void (ARM7TDMI::*Handler16)(u16);
  typedef void (ARM7TDMI::*Handler32)(u32);

private:
  friend struct TableGen;

  auto GetReg(int id) -> u32 {
    u32 result = 0;
    bool is_banked = id >= 8 && id != 15;

    if(unlikely(ldm_usermode_conflict && is_banked)) {
      result |= state.bank[BANK_NONE][id - 8];
    }

    if(likely(!cpu_mode_is_invalid || !is_banked)) {
      result |= state.reg[id];
    }

    return result;
  }

  void SetReg(int id, u32 value) {
    bool is_banked = id >= 8 && id != 15;

    if(unlikely(ldm_usermode_conflict && is_banked)) {
      state.bank[BANK_NONE][id - 8] = value;
    }

    if(likely(!cpu_mode_is_invalid || !is_banked)) {
      state.reg[id] = value;
    }
  }

  auto GetSPSR() -> StatusRegister {
    // CPSR/SPSR bit4 is forced to one on the ARM7TDMI:
    u32 spsr = 0x00000010;

    if(unlikely(ldm_usermode_conflict)) {
      /* TODO: current theory is that the value gets OR'd with CPSR,
       * because in user and system mode SPSR reads return the CPSR value.
       * But this needs to be confirmed.
       */
      spsr |= state.cpsr.v;
    }

    if(likely(!cpu_mode_is_invalid)) {
      spsr |= p_spsr->v;
    }

    return StatusRegister{spsr};
  }

  void SignalIRQ() {
    if(latch_irq_disable) {
      return;
    }

    // Prefetch the next instruction
    // The result will be discarded because we flush the pipeline.
    // But this is important for timing nonetheless.
    if(state.cpsr.f.thumb) {
      ReadHalf(state.r15 & ~1, pipe.access);
    } else {
      ReadWord(state.r15 & ~3, pipe.access);
    }

    // Save current program status register.
    state.spsr[BANK_IRQ].v = state.cpsr.v;

    // Enter IRQ mode and disable IRQs.
    SwitchMode(MODE_IRQ);
    state.cpsr.f.mask_irq = 1;

    // Save current program counter and disable Thumb.
    if(state.cpsr.f.thumb) {
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
    if(condition == COND_AL)
      return true;
    return s_condition_lut[(static_cast<int>(condition) << 4) | (state.cpsr.v >> 28)];
  }

  void ReloadPipeline16() {
    pipe.opcode[0] = bus.ReadHalf(state.r15 + 0, Access::Code | Access::Nonsequential);
    pipe.opcode[1] = bus.ReadHalf(state.r15 + 2, Access::Code | Access::Sequential);
    pipe.access = Access::Code | Access::Sequential;
    state.r15 += 4;

    latch_irq_disable = state.cpsr.f.mask_irq;
  }

  void ReloadPipeline32() {
    pipe.opcode[0] = bus.ReadWord(state.r15 + 0, Access::Code | Access::Nonsequential);
    pipe.opcode[1] = bus.ReadWord(state.r15 + 4, Access::Code | Access::Sequential);
    pipe.access = Access::Code | Access::Sequential;
    state.r15 += 8;

    latch_irq_disable = state.cpsr.f.mask_irq;
  }

  auto GetRegisterBankByMode(Mode mode) -> Bank {
    switch(mode) {
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

  void ClearLDMUsermodeConflictFlag() {
    ldm_usermode_conflict = false;
  }

  #include "handlers/arithmetic.inl"
  #include "handlers/handler16.inl"
  #include "handlers/handler32.inl"
  #include "handlers/memory.inl"

  Scheduler& scheduler;
  Bus& bus;
  StatusRegister* p_spsr;
  bool ldm_usermode_conflict;
  bool cpu_mode_is_invalid;

  struct Pipeline {
    int access;
    u32 opcode[2];
  } pipe;

  bool irq_line;
  bool latch_irq_disable;

  static std::array<bool, 256> s_condition_lut;
  static std::array<Handler16, 1024> s_opcode_lut_16;
  static std::array<Handler32, 4096> s_opcode_lut_32;
};

} // namespace nba::core::arm
