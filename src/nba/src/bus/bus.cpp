/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <nba/common/punning.hpp>
#include <stdexcept>

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"

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
  hw.waitcnt = {};
  hw.haltcnt = Hardware::HaltControl::Run;
  hw.rcnt[0] = 0;
  hw.rcnt[1] = 0;
  hw.postflg = 0;
  prefetch = {};
  dma = {};
  UpdateWaitStateTable();
}

void Bus::Attach(std::vector<u8> const& bios) {
  if (bios.size() > memory.bios.size()) {
    throw std::runtime_error("BIOS image is too big");
  }

  std::copy(bios.begin(), bios.end(), memory.bios.begin());
}

void Bus::Attach(ROM&& rom) {
  memory.rom = std::move(rom);
}

auto Bus::ReadByte(u32 address, int access) ->  u8 {
  return Read<u8>(address, access);
}

auto Bus::ReadHalf(u32 address, int access) -> u16 {
  return Read<u16>(address, access);
}

auto Bus::ReadWord(u32 address, int access) -> u32 {
  return Read<u32>(address, access);
}

void Bus::WriteByte(u32 address, u8  value, int access) {
  Write<u8>(address, access, value);
}

void Bus::WriteHalf(u32 address, u16 value, int access) {
  Write<u16>(address, access, value);
}

void Bus::WriteWord(u32 address, u32 value, int access) {
  Write<u32>(address, access, value);
}

template<typename T>
auto Bus::Read(u32 address, int access) -> T {
  auto page = address >> 24;
  auto is_u32 = std::is_same_v<T, u32>;

  switch (page) {
    // BIOS
    case 0x00: {
      Step(1);
      return ReadBIOS(Align<T>(address));
    }
    // EWRAM (external work RAM)
    case 0x02: {
      Step(is_u32 ? 6 : 3);
      return read<T>(memory.wram.data(), Align<T>(address) & 0x3FFFF);
    }
    // IWRAM (internal work RAM)
    case 0x03: {
      Step(1);
      return read<T>(memory.iram.data(), Align<T>(address) & 0x7FFF);
    }
    // MMIO
    case 0x04: {
      Step(1);
      address = Align<T>(address);
      if constexpr(std::is_same_v<T,  u8>) return hw.ReadByte(address);
      if constexpr(std::is_same_v<T, u16>) return hw.ReadHalf(address);
      if constexpr(std::is_same_v<T, u32>) return hw.ReadWord(address);
      return 0;
    }
    // PRAM (palette RAM)
    case 0x05: {
      Step(is_u32 ? 2 : 1);
      return hw.ppu.ReadPRAM<T>(Align<T>(address));
    }
    // VRAM (video RAM)
    case 0x06: {
      Step(is_u32 ? 2 : 1);
      return hw.ppu.ReadVRAM<T>(Align<T>(address));
    }
    // OAM (object attribute map)
    case 0x07: {
      Step(1);
      return hw.ppu.ReadOAM<T>(Align<T>(address));
    }
    // ROM (WS0, WS1, WS2)
    case 0x08 ... 0x0D: {
      address = Align<T>(address);

      auto sequential = access & Sequential;
      bool code = access & Code;

      if ((address & 0x1'FFFF) == 0) {
        sequential = 0;
      }

      if constexpr(std::is_same_v<T,  u8>) {
        auto shift = ((address & 1) << 3);
        Prefetch(address, code, wait16[sequential][page]);
        return memory.rom.ReadROM16(address) >> shift;
      }

      if constexpr(std::is_same_v<T, u16>) {
        Prefetch(address, code, wait16[sequential][page]);
        return memory.rom.ReadROM16(address);
      }

      if constexpr(std::is_same_v<T, u32>) {
        Prefetch(address, code, wait32[sequential][page]);
        return memory.rom.ReadROM32(address);  
      }

      return 0;
    }
    // SRAM or FLASH backup
    case 0x0E ... 0x0F: {
      StopPrefetch();
      Step(wait16[0][0xE]);

      u32 value = memory.rom.ReadSRAM(address);

      if constexpr(std::is_same_v<T, u16>) value *= 0x0101;
      if constexpr(std::is_same_v<T, u32>) value *= 0x01010101;

      return T(value);
    }
    // Unmapped memory
    default: {
      Step(1);
      return ReadOpenBus(Align<T>(address));
    }
  }  

  return 0;
}

template<typename T>
void Bus::Write(u32 address, int access, T value) {
  auto page = address >> 24;
  auto is_u32 = std::is_same_v<T, u32>;

  switch (page) {
    // EWRAM (external work RAM)
    case 0x02: {
      Step(is_u32 ? 6 : 3);
      write<T>(memory.wram.data(), Align<T>(address) & 0x3FFFF, value);
      break;
    }
    // IWRAM (internal work RAM)
    case 0x03: {
      Step(1);
      write<T>(memory.iram.data(), Align<T>(address) & 0x7FFF,  value);
      break;
    }
    // MMIO
    case 0x04: {
      Step(1);
      address = Align<T>(address);
      if constexpr(std::is_same_v<T,  u8>) hw.WriteByte(address, value);
      if constexpr(std::is_same_v<T, u16>) hw.WriteHalf(address, value);
      if constexpr(std::is_same_v<T, u32>) hw.WriteWord(address, value);
      break;
    }
    // PRAM (palette RAM)
    case 0x05: {
      Step(is_u32 ? 2 : 1);
      hw.ppu.WritePRAM<T>(Align<T>(address), value);
      break;
    }
    // VRAM (video RAM)
    case 0x06: {
      Step(is_u32 ? 2 : 1);
      hw.ppu.WriteVRAM<T>(Align<T>(address), value);
      break;
    }
    // OAM (object attribute map)
    case 0x07: {
      Step(1);
      hw.ppu.WriteOAM<T>(Align<T>(address), value);
      break;
    }
    // ROM (WS0, WS1, WS2)
    case 0x08 ... 0x0D: {
      address = Align<T>(address);

      auto sequential = access & Sequential;

      if ((address & 0x1'FFFF) == 0) {
        sequential = 0;
      }

      StopPrefetch();

      // TODO: figure out how 8-bit and 32-bit accesses actually work.
      if constexpr(std::is_same_v<T, u8>) {
        Step(wait16[sequential][page]);
        memory.rom.WriteROM(address, value * 0x0101);
      }

      if constexpr(std::is_same_v<T, u16>) {
        Step(wait16[sequential][page]);
        memory.rom.WriteROM(address, value);
      }

      if constexpr(std::is_same_v<T, u32>) {
        Step(wait32[sequential][page]);
        memory.rom.WriteROM(address|0, value & 0xFFFF);
        memory.rom.WriteROM(address|2, value >> 16);
      }
      break;
    }
    // SRAM or FLASH backup
    case 0x0E ... 0x0F: {
      StopPrefetch();
      Step(wait16[0][0xE]);

      if constexpr(std::is_same_v<T, u16>) value >>= (address & 1) << 3;
      if constexpr(std::is_same_v<T, u32>) value >>= (address & 3) << 3;

      memory.rom.WriteSRAM(address, u8(value));
      break;
    }
    // Unmapped memory
    default: {
      Step(1);
      break;
    }
  }
}

auto Bus::ReadBIOS(u32 address) -> u32 {
  if (address >= 0x4000) {
    return ReadOpenBus(address);
  }

  auto shift = (address & 3) << 3;
  if (hw.cpu.state.r15 < 0x4000) {
    address &= ~3;
    memory.latch.bios = read<u32>(memory.bios.data(), address);
  } else {
    Log<Trace>("Bus: illegal BIOS read: 0x{:08X}", address);
  }
  return memory.latch.bios >> shift;
}

auto Bus::ReadOpenBus(u32 address) -> u32 {
  u32 word = 0;
  auto& cpu = hw.cpu;
  auto shift = (address & 3) << 3;

  Log<Trace>("Bus: illegal memory read: 0x{:08X}", address);

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
        word  = cpu.GetFetchedOpcode(1);
        word |= (word << 16);
        break;
      }
      // BIOS, OAM (32-bit)
      case 0x00:
      case 0x07: {
        if ((r15 & 2) == 0) {
          word  = cpu.GetFetchedOpcode(0);
          word |= cpu.GetFetchedOpcode(1) << 16;
        } else {
          // TODO: this should be LSW=$+4 MSW=$+6
          // Unfortunately $+6 has not been fetched at this point. 
          word  = cpu.GetFetchedOpcode(1);
          word |= (word << 16);
        }
        break;
      }
      // IWRAM bus (16-bit special-case)
      case 0x03: {
        if ((r15 & 2) == 0) {
          word  = cpu.GetFetchedOpcode(0);
          word |= cpu.GetFetchedOpcode(1) << 16;
        } else {
          word  = cpu.GetFetchedOpcode(1);
          word |= cpu.GetFetchedOpcode(0) << 16;
        }
        break;
      }
    }
  } else {
    word = cpu.GetFetchedOpcode(1);
  }

  return word >> shift;
}

auto Bus::GetHostAddress(u32 address, size_t size) -> u8* {
  auto& bios = memory.bios;
  auto& wram = memory.wram;
  auto& iram = memory.iram;
  auto& rom = memory.rom.GetRawROM();

  auto page = address >> 24;

  switch (page) {
    // BIOS
    case 0x00: {
      auto offset = address & 0x00FF'FFFF;
      if (offset + size <= bios.size()) {
        return bios.data() + offset;
      }
      break;
    }
    // EWRAM (external work RAM)
    case 0x02: {
      auto offset = address & 0x00FF'FFFF;
      if (offset + size <= wram.size()) {
        return wram.data() + offset;
      }
      break;
    }
    // IWRAM (internal work RAM)
    case 0x03: {
      auto offset = address & 0x00FF'FFFF;
      if (offset + size <= iram.size()) {
        return iram.data() + offset;
      }
      break;
    }
    // ROM (WS0, WS1, WS2)
    case 0x08 ... 0x0D: {
      auto offset = address & 0x01FF'FFFF;
      if (offset + size <= rom.size()) {
        return rom.data() + offset;
      }
      break;
    }
  }

  Assert(false, "Bus: cannot get host address for 0x{:08X} ({} bytes)", address, size);
  return nullptr;
}

} // namespace nba::core
