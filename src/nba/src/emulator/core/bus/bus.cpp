/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "bus.hpp"
#include "common/punning.hpp"
#include "emulator/core/cpu.hpp"

namespace nba::core {

Bus::Bus(Scheduler& scheduler, Hardware&& hw)
    : scheduler(scheduler)
    , hw(hw) {
  this->hw.bus = this;
  memory.bios.fill(0);
  Reset();
}

void Bus::Reset() {
  memory.wram.fill(0);
  memory.iram.fill(0);
  memory.latch = {};
  dma = {};
}

auto Bus::ReadByte(u32 address, Access access) ->  u8 {
  return Read<u8>(address, access);
}
auto Bus::ReadHalf(u32 address, Access access) -> u16 {
  return Read<u16>(address, access);
}
auto Bus::ReadWord(u32 address, Access access) -> u32 {
  return Read<u32>(address, access);
}

void Bus::WriteByte(u32 address, u8  value, Access access) {
  Write<u8>(address, access, value);
}
void Bus::WriteHalf(u32 address, u16 value, Access access) {
  Write<u16>(address, access, value);
}
void Bus::WriteWord(u32 address, u32 value, Access access) {
  Write<u32>(address, access, value);
}

void Bus::Idle() {
  // TODO
  PrefetchStepRAM(1);
}

template<typename T> auto Bus::Read(u32 address, Access access) -> T {
  int cycles;
  int page = address >> 24;

  // TODO: optimize this
  if (page != 0x0E && page != 0x0F) {
    address &= ~(sizeof(T) - 1);
  }

  // TODO: optimize this
  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  // TODO: optimize this
  if constexpr (std::is_same_v<T, u32>) {
    cycles = cycles32[int(access)][page];
  } else {
    cycles = cycles16[int(access)][page];
  }

  switch (page) {
    // BIOS
    case 0x00: {
      PrefetchStepRAM(cycles);
      return ReadBIOS(address);
    }
    // EWRAM (external work RAM)
    case 0x02: {
      PrefetchStepRAM(cycles);
      return common::read<T>(memory.wram.data(), address & 0x3FFFF);
    }
    // IWRAM (internal work RAM)
    case 0x03: {
      PrefetchStepRAM(cycles);
      return common::read<T>(memory.iram.data(), address & 0x7FFF);
    }
    // MMIO
    case 0x04: {
      PrefetchStepRAM(cycles);

      if constexpr (std::is_same_v<T, u32>) {
        return hw.ReadWord(address);
      }

      if constexpr (std::is_same_v<T, u16>) {
        return hw.ReadHalf(address);
      }

      return hw.ReadByte(address);
    }
    // PRAM (palette RAM)
    case 0x05: {
      PrefetchStepRAM(cycles);
      return hw.ppu.ReadPRAM<T>(address);
    }
    // VRAM (video RAM)
    case 0x06: {
      PrefetchStepRAM(cycles);
      return hw.ppu.ReadVRAM<T>(address);
    }
    // OAM (object attribute map)
    case 0x07: {
      PrefetchStepRAM(cycles);
      return hw.ppu.ReadOAM<T>(address);
    }
    // ROM
    case 0x08 ... 0x0D: {
      PrefetchStepROM(address, cycles);

      if constexpr (std::is_same_v<T,  u8>) {
        return memory.game_pak.ReadROM16(address) >> ((address & 1) << 3);
      }

      if constexpr (std::is_same_v<T, u16>) {
        return memory.game_pak.ReadROM16(address);
      }

      return memory.game_pak.ReadROM32(address);
    }
    // SRAM or FLASH backup
    case 0x0E ... 0x0F: {
      PrefetchStepROM(address, cycles);

      u32 value = memory.game_pak.ReadSRAM(address);

      if constexpr (std::is_same_v<T, u16>) {
        value *= 0x0101;
      }

      if constexpr (std::is_same_v<T, u32>) {
        value *= 0x01010101;
      }

      return T(value);
    }
    // Unmapped memory
    default: {
      PrefetchStepRAM(cycles);
      return ReadOpenBus(address);
    }
  }  

  return 0;
}

template<typename T> void Bus::Write(u32 address, Access access, T value) {
  int cycles;
  int page = address >> 24;

  // TODO: optimiize this
  if (page != 0x0E && page != 0x0F) {
    address &= ~(sizeof(T) - 1);
  }

  // TODO: optimize this
  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  // TODO: optimize this
  if (std::is_same_v<T, u32>) {
    cycles = cycles32[int(access)][page];
  } else {
    cycles = cycles16[int(access)][page];
  }

  switch (page) {
    // EWRAM (external work RAM)
    case 0x02: {
      PrefetchStepRAM(cycles);
      common::write<T>(memory.wram.data(), address & 0x3FFFF, value);
      break;
    }
    // IWRAM (internal work RAM)
    case 0x03: {
      PrefetchStepRAM(cycles);
      common::write<T>(memory.iram.data(), address & 0x7FFF,  value);
      break;
    }
    // MMIO
    case 0x04: {
      PrefetchStepRAM(cycles);

      if constexpr (std::is_same_v<T, u32>) {
        hw.WriteWord(address, value);
      }

      if constexpr (std::is_same_v<T, u16>) {
        hw.WriteHalf(address, value);
      }

      if constexpr (std::is_same_v<T, u8>) {
        hw.WriteByte(address, value);
      }
      break;
    }
    // PRAM (palette RAM)
    case 0x05: {
      PrefetchStepRAM(cycles);
      hw.ppu.WritePRAM<T>(address, value);
      break;
    }
    // VRAM (video RAM)
    case 0x06: {
      PrefetchStepRAM(cycles);
      hw.ppu.WriteVRAM<T>(address, value);
      break;
    }
    // OAM (object attribute map)
    case 0x07: {
      PrefetchStepRAM(cycles);
      hw.ppu.WriteOAM<T>(address, value);
      break;
    }
    // ROM
    case 0x08 ... 0x0D: {
      PrefetchStepROM(address, cycles);

      // TODO: figure out how 8-bit and 16-bit accesses actually work.
      if constexpr (std::is_same_v<T, u8>) {
        memory.game_pak.WriteROM(address, value * 0x0101);
      }

      if constexpr (std::is_same_v<T, u16>) {
        memory.game_pak.WriteROM(address, value);
      }

      if constexpr (std::is_same_v<T, u32>) {
        memory.game_pak.WriteROM(address|0, value & 0xFFFF);
        memory.game_pak.WriteROM(address|2, value >> 16);
      }
      break;
    }
    // SRAM or FLASH backup
    case 0x0E ... 0x0F: {
      PrefetchStepROM(address, cycles);
        
      if constexpr (std::is_same_v<T, u32>) {
        value >>= (address & 3) << 3;
      }

      if constexpr (std::is_same_v<T, u16>) {
        value >>= (address & 1) << 3;
      }

      memory.game_pak.WriteSRAM(address, u8(value));
      break;
    }
    // Unmapped memory
    default: {
      PrefetchStepRAM(cycles);
      break;
    }
  }
}

void Bus::PrefetchStepROM(u32 address, int cycles) {
  // TODO (where does this belong?)
  PrefetchStepRAM(cycles);
}

void Bus::PrefetchStepRAM(int cycles) {
  // TODO (where does this belong?)
  dma.openbus = false;

  if (hw.dma.IsRunning() && !dma.active) {
    dma.active = true;
    hw.dma.Run();
    dma.active = false;
    dma.openbus = true;
  }

  scheduler.AddCycles(cycles);
}

void Bus::UpdateWaitStateTable() {
  // TODO (where does this belong?)
  static constexpr int nseq[4] = { 5, 4, 3, 9 };
  static constexpr int seq0[2] = { 3, 2 };
  static constexpr int seq1[2] = { 5, 2 };
  static constexpr int seq2[2] = { 9, 2 };

  auto n = int(Access::Nonsequential);
  auto s = int(Access::Sequential);
  auto& waitcnt = hw.cpu.mmio.waitcnt;
  auto sram = nseq[waitcnt.sram];

  for (int i = 0; i < 2; i++) {
    // ROM: WS0/WS1/WS2 16-bit non-sequential access
    cycles16[n][0x8 + i] = nseq[waitcnt.ws0_n];
    cycles16[n][0xA + i] = nseq[waitcnt.ws1_n];
    cycles16[n][0xC + i] = nseq[waitcnt.ws2_n];

    // ROM: WS0/WS1/WS2 16-bit sequential access
    cycles16[s][0x8 + i] = seq0[waitcnt.ws0_s];
    cycles16[s][0xA + i] = seq1[waitcnt.ws1_s];
    cycles16[s][0xC + i] = seq2[waitcnt.ws2_s];

    // ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access
    cycles32[n][0x8 + i] = cycles16[n][0x8] + cycles16[s][0x8];
    cycles32[n][0xA + i] = cycles16[n][0xA] + cycles16[s][0xA];
    cycles32[n][0xC + i] = cycles16[n][0xC] + cycles16[s][0xC];

    // ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses
    cycles32[s][0x8 + i] = cycles16[s][0x8] * 2;
    cycles32[s][0xA + i] = cycles16[s][0xA] * 2;
    cycles32[s][0xC + i] = cycles16[s][0xC] * 2;

    // SRAM
    cycles16[n][0xE + i] = sram;
    cycles32[n][0xE + i] = sram;
    cycles16[s][0xE + i] = sram;
    cycles32[s][0xE + i] = sram;
  }
}

auto Bus::ReadBIOS(u32 address) -> u32 {
  auto shift = (address & 3) << 3;

  address &= ~3;

  if (address >= 0x4000) {
    return ReadOpenBus(address) >> shift;
  }

  if (hw.cpu.state.r15 >= 0x4000) {
    return memory.latch.bios >> shift;
  }
  
  return (memory.latch.bios = common::read<u32>(memory.bios.data(), address)) >> shift;
}

auto Bus::ReadOpenBus(u32 address) -> u32 {
  u32 word = 0;
  auto& cpu = hw.cpu;
  auto shift = (address & 3) << 3;

  if (hw.dma.IsRunning() || dma.openbus) {
    return hw.dma.GetOpenBusValue() >> shift;
  }

  if (cpu.state.cpsr.f.thumb) {
    auto r15 = cpu.state.r15;

    switch (r15 >> 24) {
      // EWRAM, PRAM, VRAM, ROM (16-bit)
      case 0x02:
      case 0x05 ... 0x06:
      case 0x08 ... 0x0D: {
        word = cpu.GetPrefetchedOpcode(1) * 0x00010001;
        break;
      }
      // BIOS, OAM (32-bit)
      case 0x00:
      case 0x07: {
        if (r15 & 3) {
          word = cpu.GetPrefetchedOpcode(0) |
                (cpu.GetPrefetchedOpcode(1) << 16);
        } else {
          word = cpu.GetPrefetchedOpcode(1) * 0x00010001;
        }
        break;
      }
      // IWRAM bus (16-bit special-case)
      case 0x03: {
        if (r15 & 3) {
          word = cpu.GetPrefetchedOpcode(0) |
                (cpu.GetPrefetchedOpcode(1) << 16);
        } else {
          word = cpu.GetPrefetchedOpcode(1) |
                (cpu.GetPrefetchedOpcode(0) << 16);
        }
        break;
      }
    }
  } else {
    word = cpu.GetPrefetchedOpcode(1);
  }

  return word >> shift;
}

} // namespace nba::core
