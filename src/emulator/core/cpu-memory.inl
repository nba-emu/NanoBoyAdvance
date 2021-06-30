/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

inline u32 CPU::ReadBIOS(u32 address) {
  int shift = (address & 3) * 8;

  address &= ~3;

  if (address >= 0x4000) {
    return ReadUnused(address) >> shift;
  }

  if (state.r15 >= 0x4000) {
    return memory.bios_latch >> shift;
  }

  memory.bios_latch = common::read<u32>(memory.bios, address);

  return memory.bios_latch >> shift;
}

inline u32 CPU::ReadUnused(u32 address) {
  u32 result = 0;

  if (dma.IsRunning() || openbus_from_dma) {
    return dma.GetOpenBusValue() >> ((address & 3) * 8);
  }

  if (state.cpsr.f.thumb) {
    auto r15 = state.r15;

    switch (r15 >> 24) {
      case REGION_EWRAM:
      case REGION_PRAM:
      case REGION_VRAM:
      case REGION_ROM_W0_L:
      case REGION_ROM_W0_H:
      case REGION_ROM_W1_L:
      case REGION_ROM_W1_H:
      case REGION_ROM_W2_L:
      case REGION_ROM_W2_H: {
        result = GetPrefetchedOpcode(1) * 0x00010001;
        break;
      }
      case REGION_BIOS:
      case REGION_OAM: {
        if (r15 & 3) {
          result = GetPrefetchedOpcode(0) |
                  (GetPrefetchedOpcode(1) << 16);
        } else {
          // TODO: this is incorrect, unfortunately [$+6] has not been prefetched yet.
          result = GetPrefetchedOpcode(1) * 0x00010001;
        }
        break;
      }
      case REGION_IWRAM: {
        if (r15 & 3) {
          result = GetPrefetchedOpcode(0) |
                  (GetPrefetchedOpcode(1) << 16);
        } else {
          result = GetPrefetchedOpcode(1) |
                  (GetPrefetchedOpcode(0) << 16);
        }
        break;
      }
    }
  } else {
    result = GetPrefetchedOpcode(1);
  }

  return result >> ((address & 3) * 8);
}

template<typename T>
auto CPU::Read(u32 address, Access access) -> T {
  int cycles;
  int page = address >> 24;

  if (page != REGION_SRAM_1 && page != REGION_SRAM_2) {
    address &= ~(sizeof(T) - 1);
  }

  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  if (std::is_same_v<T, u32>) {
    cycles = cycles32[int(access)][page];
  } else {
    cycles = cycles16[int(access)][page];
  }

  switch (page) {
    case REGION_BIOS: {
      PrefetchStepRAM(cycles);
      return ReadBIOS(address);
    }
    case REGION_EWRAM: {
      PrefetchStepRAM(cycles);
      return common::read<T>(memory.wram, address & 0x3FFFF);
    }
    case REGION_IWRAM: {
      PrefetchStepRAM(cycles);
      return common::read<T>(memory.iram, address & 0x7FFF);
    }
    case REGION_MMIO: {
      PrefetchStepRAM(cycles);
      if constexpr (std::is_same_v<T, u32>) {
        return (ReadMMIO(address + 0) <<  0) |
               (ReadMMIO(address + 1) <<  8) |
               (ReadMMIO(address + 2) << 16) |
               (ReadMMIO(address + 3) << 24);
      }
      if constexpr (std::is_same_v<T, u16>) {
        return (ReadMMIO(address + 0) << 0) |
               (ReadMMIO(address + 1) << 8);
      }
      return ReadMMIO(address);
    }
    case REGION_PRAM: {
      PrefetchStepRAM(cycles);
      return ppu.ReadPRAM<T>(address);
    }
    case REGION_VRAM: {
      PrefetchStepRAM(cycles);
      return ppu.ReadVRAM<T>(address);
    }
    case REGION_OAM: {
      PrefetchStepRAM(cycles);
      return ppu.ReadOAM<T>(address);
    }
    case REGION_ROM_W0_L:
    case REGION_ROM_W0_H:
    case REGION_ROM_W1_L:
    case REGION_ROM_W1_H:
    case REGION_ROM_W2_L:
    case REGION_ROM_W2_H: {
      PrefetchStepROM(address, cycles);
      if constexpr (std::is_same_v<T,  u8>) {
        return game_pak.ReadROM16(address) >> ((address & 1) << 3);
      }
      if constexpr (std::is_same_v<T, u16>) {
        return game_pak.ReadROM16(address);
      }
      if constexpr (std::is_same_v<T, u32>) {
        return game_pak.ReadROM32(address);
      }
      break;
    }
    case REGION_SRAM_1:
    case REGION_SRAM_2: {
      PrefetchStepROM(address, cycles);
      u32 value = game_pak.ReadSRAM(address);
      if (std::is_same_v<T, u16>) value *= 0x0101;
      if (std::is_same_v<T, u32>) value *= 0x01010101;
      return T(value);
    }
    default: {
      PrefetchStepRAM(cycles);
      return ReadUnused(address);
    }
  }  

  return 0;
}

template<typename T>
void CPU::Write(u32 address, T value, Access access) {
  int cycles;
  int page = address >> 24;

  if (page != REGION_SRAM_1 && page != REGION_SRAM_2) {
    address &= ~(sizeof(T) - 1);
  }

  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  if (std::is_same_v<T, u32>) {
    cycles = cycles32[int(access)][page];
  } else {
    cycles = cycles16[int(access)][page];
  }

  switch (page) {
    case REGION_EWRAM: {
      PrefetchStepRAM(cycles);
      common::write<T>(memory.wram, address & 0x3FFFF, value);
      break;
    }
    case REGION_IWRAM: {
      PrefetchStepRAM(cycles);
      common::write<T>(memory.iram, address & 0x7FFF,  value);
      break;
    }
    case REGION_MMIO: {
      PrefetchStepRAM(cycles);
      if constexpr (std::is_same_v<T, u32>) {
        WriteMMIO16(address + 0, value & 0xFFFF);
        WriteMMIO16(address + 2, value >> 16);
      }
      if constexpr (std::is_same_v<T, u16>) {
        WriteMMIO16(address, value);
      }
      if constexpr (std::is_same_v<T, u8>) {
        WriteMMIO(address, value & 0xFF);
      }
      break;
    }
    case REGION_PRAM: {
      PrefetchStepRAM(cycles);
      ppu.WritePRAM<T>(address, value);
      break;
    }
    case REGION_VRAM: {
      PrefetchStepRAM(cycles);
      ppu.WriteVRAM<T>(address, value);
      break;
    }
    case REGION_OAM: {
      PrefetchStepRAM(cycles);
      ppu.WriteOAM<T>(address, value);
      break;
    }
    case REGION_ROM_W0_L:
    case REGION_ROM_W0_H:
    case REGION_ROM_W1_L:
    case REGION_ROM_W1_H:
    case REGION_ROM_W2_L:
    case REGION_ROM_W2_H: {
      // TODO: figure out how 8-bit and 16-bit accesses actually work.
      PrefetchStepROM(address, cycles);
      if constexpr (std::is_same_v<T, u8>) {
        game_pak.WriteROM(address, value * 0x0101);
      }
      if constexpr (std::is_same_v<T, u16>) {
        game_pak.WriteROM(address, value);
      }
      if constexpr (std::is_same_v<T, u32>) {
        game_pak.WriteROM(address|0, value & 0xFFFF);
        game_pak.WriteROM(address|2, value >> 16);
      }
      break;
    }
    case REGION_SRAM_1:
    case REGION_SRAM_2: {
      PrefetchStepROM(address, cycles);
        
      if constexpr (std::is_same_v<T, u32>) value >>= (address & 3) << 3;
      if constexpr (std::is_same_v<T, u16>) value >>= (address & 1) << 3;

      game_pak.WriteSRAM(address, u8(value));
      break;
    }
    default: {
      PrefetchStepRAM(cycles);
    }
  }
}
