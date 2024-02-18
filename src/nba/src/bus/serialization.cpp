/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "bus/bus.hpp"

namespace nba::core {

void Bus::LoadState(SaveState const& state) {
  memory.wram = state.bus.memory.wram;
  memory.iram = state.bus.memory.iram;
  memory.latch.bios = state.bus.memory.latch.bios;
  memory.rom.LoadState(state);

  hw.waitcnt.sram = state.bus.io.waitcnt.sram;
  for(int i = 0; i < 2; i++) {
    hw.waitcnt.ws0[i] = state.bus.io.waitcnt.ws0[i];
    hw.waitcnt.ws1[i] = state.bus.io.waitcnt.ws1[i];
    hw.waitcnt.ws2[i] = state.bus.io.waitcnt.ws2[i];
  }
  hw.waitcnt.phi = state.bus.io.waitcnt.phi;
  hw.waitcnt.prefetch = state.bus.io.waitcnt.prefetch;
  UpdateWaitStateTable();

  hw.haltcnt = (Hardware::HaltControl)state.bus.io.haltcnt;
  hw.rcnt[0] = state.bus.io.rcnt[0];
  hw.rcnt[1] = state.bus.io.rcnt[1];
  hw.postflg = state.bus.io.postflg;
  hw.prefetch_buffer_was_disabled = state.bus.prefetch_buffer_was_disabled;

  prefetch.active = state.bus.prefetch.active;
  prefetch.head_address = state.bus.prefetch.head_address;
  prefetch.count = state.bus.prefetch.count;
  prefetch.countdown = state.bus.prefetch.countdown;
  prefetch.thumb = state.bus.prefetch.thumb;
  if(prefetch.thumb) {
    prefetch.opcode_width = sizeof(u16);
    prefetch.capacity = 8;
    prefetch.duty = wait16[int(Access::Sequential)][prefetch.last_address >> 24];
  } else {
    prefetch.opcode_width = sizeof(u32);
    prefetch.capacity = 4;
    prefetch.duty = wait32[int(Access::Sequential)][prefetch.last_address >> 24];
  }

  last_access = state.bus.last_access;

  parallel_internal_cpu_cycle_limit = state.bus.parallel_internal_cpu_cycle_limit;
}

void Bus::CopyState(SaveState& state) {
  state.bus.memory.wram = memory.wram;
  state.bus.memory.iram = memory.iram;
  state.bus.memory.latch.bios = memory.latch.bios;
  memory.rom.CopyState(state);

  state.bus.io.waitcnt.sram = hw.waitcnt.sram;
  for(int i = 0; i < 2; i++) {
    state.bus.io.waitcnt.ws0[i] = hw.waitcnt.ws0[i];
    state.bus.io.waitcnt.ws1[i] = hw.waitcnt.ws1[i];
    state.bus.io.waitcnt.ws2[i] = hw.waitcnt.ws2[i];
  }
  state.bus.io.waitcnt.phi = hw.waitcnt.phi;
  state.bus.io.waitcnt.prefetch = hw.waitcnt.prefetch;

  state.bus.io.haltcnt = (int)hw.haltcnt;
  state.bus.io.rcnt[0] = hw.rcnt[0];
  state.bus.io.rcnt[1] = hw.rcnt[1];
  state.bus.io.postflg = hw.postflg;
  state.bus.prefetch_buffer_was_disabled = hw.prefetch_buffer_was_disabled;

  state.bus.prefetch.active = prefetch.active;
  state.bus.prefetch.head_address = prefetch.head_address;
  state.bus.prefetch.count = prefetch.count;
  state.bus.prefetch.countdown = prefetch.countdown;
  state.bus.prefetch.thumb = prefetch.thumb;

  state.bus.last_access = last_access;

  state.bus.parallel_internal_cpu_cycle_limit = parallel_internal_cpu_cycle_limit;
}

} // namespace nba::core