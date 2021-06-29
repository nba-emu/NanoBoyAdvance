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

  memory.bios_latch = Read<u32>(memory.bios, address);

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
auto CPU::Read_(u32 address, Access access) -> T {
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
      return Read<T>(memory.wram, address & 0x3FFFF);
    }
    case REGION_IWRAM: {
      PrefetchStepRAM(cycles);
      return Read<T>(memory.iram, address & 0x7FFF);
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
      return Read<T>(ppu.pram, address & 0x3FF);
    }
    case REGION_VRAM: {
      PrefetchStepRAM(cycles);
      address &= 0x1FFFF;
      if (address >= 0x18000)
        address &= ~0x8000;
      return Read<T>(ppu.vram, address);
    }
    case REGION_OAM: {
      PrefetchStepRAM(cycles);
      return Read<T>(ppu.oam, address & 0x3FF);
    }
    case REGION_ROM_W0_L:
    case REGION_ROM_W0_H:
    case REGION_ROM_W1_L:
    case REGION_ROM_W1_H:
    case REGION_ROM_W2_L:
    case REGION_ROM_W2_H: {
      PrefetchStepROM(address, cycles);
      if constexpr (std::is_same_v<T,  u8>) {
        return game_pak.ReadROM(address) >> ((address & 1) << 3);
      }
      if constexpr (std::is_same_v<T, u16>) {
        return game_pak.ReadROM(address);
      }
      if constexpr (std::is_same_v<T, u32>) {
        auto lsw = game_pak.ReadROM(address|0);
        auto msw = game_pak.ReadROM(address|2);
        return (msw << 16) | lsw;
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
void CPU::Write_(u32 address, T value, Access access) {
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
      Write<T>(memory.wram, address & 0x3FFFF, value);
      break;
    }
    case REGION_IWRAM: {
      PrefetchStepRAM(cycles);
      Write<T>(memory.iram, address & 0x7FFF,  value);
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
      if constexpr (std::is_same_v<T, u8>) {
        Write<u16>(ppu.pram, address & 0x3FE, value * 0x0101);
      } else {
        Write<T>(ppu.pram, address & 0x3FF, value);
      }
      break;
    }
    case REGION_VRAM: {
      PrefetchStepRAM(cycles);
      address &= 0x1FFFF;
      if (address >= 0x18000) {
        address &= ~0x8000;
      }
      if (std::is_same_v<T, u8>) {
        // TODO: move logic to decide the writeable area to the PPU class.
        auto limit = ppu.mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;

        if (address < limit) {
          Write<u16>(ppu.vram, address & ~1, value * 0x0101);
        }
      } else {
        Write<T>(ppu.vram, address, value);
      }
      break;
    }
    case REGION_OAM: {
      PrefetchStepRAM(cycles);
      if constexpr (!std::is_same_v<T, u8>) {
        Write<T>(ppu.oam, address & 0x3FF, value);
      }
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
