/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

template<bool debug>
inline u32 CPU::ReadBIOS(u32 address) {
  int shift = (address & 3) * 8;

  address &= ~3;

  if (address >= 0x4000) {
    return ReadUnused(address) >> shift;
  }

  if (state.r15 >= 0x4000) {
    return memory.bios_latch >> shift;
  }

  auto word = read<u32>(memory.bios, address);

  if constexpr (!debug) {
    memory.bios_latch = word;
  }

  return word >> shift;
}

inline u32 CPU::ReadUnused(u32 address) {
  u32 result = 0;

  if (dma.IsRunning() || openbus_from_dma) {
    return dma.GetOpenBusValue() >> ((address & 3) * 8);
  }

  if (state.cpsr.f.thumb) {
    auto r15 = state.r15;

    switch (r15 >> 24) {
      // 16-bit busses (minus IWRAM)
      case 0x02:
      case 0x05 ... 0x06:
      case 0x08 ... 0x0D: {
        result = GetPrefetchedOpcode(1) * 0x00010001;
        break;
      }
      // 32-bit busses
      case 0x00:
      case 0x07: {
        if (r15 & 3) {
          result = GetPrefetchedOpcode(0) |
                  (GetPrefetchedOpcode(1) << 16);
        } else {
          // TODO: this is incorrect, unfortunately [$+6] has not been prefetched yet.
          result = GetPrefetchedOpcode(1) * 0x00010001;
        }
        break;
      }
      // 16-bit IWRAM bus (works different from other 16-bit busses)
      case 0x03: {
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

template<typename T, bool debug>
auto CPU::Read(u32 address, Access access) -> T {
  int cycles;
  int page = address >> 24;

  if (page != 0x0E && page != 0x0F) {
    address &= ~(sizeof(T) - 1);
  }

  if constexpr (!debug) {
    if ((address & 0x1FFFF) == 0) {
      access = Access::Nonsequential;
    }
    
    if constexpr (std::is_same_v<T, u32>) {
      cycles = cycles32[int(access)][page];
    } else {
      cycles = cycles16[int(access)][page];
    }
  }

  switch (page) {
    case 0x00: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return ReadBIOS<debug>(address);
    }
    case 0x02: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return read<T>(memory.wram, address & 0x3FFFF);
    }
    case 0x03: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return read<T>(memory.iram, address & 0x7FFF);
    }
    case 0x04: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }

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
    case 0x05: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return ppu.ReadPRAM<T>(address);
    }
    case 0x06: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return ppu.ReadVRAM<T>(address);
    }
    case 0x07: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return ppu.ReadOAM<T>(address);
    }
    case 0x08 ... 0x0D: {
      if constexpr (!debug) {
        PrefetchStepROM(address, cycles);
      }

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
    case 0x0E ... 0x0F: {
      if constexpr (!debug) {
        PrefetchStepROM(address, cycles);
      }

      u32 value = game_pak.ReadSRAM(address);

      if constexpr (std::is_same_v<T, u16>) {
        value *= 0x0101;
      }

      if constexpr (std::is_same_v<T, u32>) {
        value *= 0x01010101;
      }

      return T(value);
    }
    default: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      return ReadUnused(address);
    }
  }  

  return 0;
}

template<typename T, bool debug>
void CPU::Write(u32 address, T value, Access access) {
  int cycles;
  int page = address >> 24;

  if (page != 0x0E && page != 0x0F) {
    address &= ~(sizeof(T) - 1);
  }

  if constexpr (!debug) {
    if ((address & 0x1FFFF) == 0) {
      access = Access::Nonsequential;
    }
    
    if (std::is_same_v<T, u32>) {
      cycles = cycles32[int(access)][page];
    } else {
      cycles = cycles16[int(access)][page];
    }
  }

  switch (page) {
    case 0x02: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      write<T>(memory.wram, address & 0x3FFFF, value);
      break;
    }
    case 0x03: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      write<T>(memory.iram, address & 0x7FFF,  value);
      break;
    }
    case 0x04: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }

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
    case 0x05: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      ppu.WritePRAM<T>(address, value);
      break;
    }
    case 0x06: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      ppu.WriteVRAM<T>(address, value);
      break;
    }
    case 0x07: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
      ppu.WriteOAM<T>(address, value);
      break;
    }
    case 0x08 ... 0x0D: {
      if constexpr (!debug) {
        PrefetchStepROM(address, cycles);
      }

      // TODO: figure out how 8-bit and 16-bit accesses actually work.
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
    case 0x0E ... 0x0F: {
      if constexpr (!debug) {
        PrefetchStepROM(address, cycles);
      }
        
      if constexpr (std::is_same_v<T, u32>) {
        value >>= (address & 3) << 3;
      }
      if constexpr (std::is_same_v<T, u16>) {
        value >>= (address & 1) << 3;
      }

      game_pak.WriteSRAM(address, u8(value));
      break;
    }
    default: {
      if constexpr (!debug) {
        PrefetchStepRAM(cycles);
      }
    }
  }
}
