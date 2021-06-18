/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

inline std::uint32_t CPU::ReadBIOS(std::uint32_t address) {
  int shift = (address & 3) * 8;

  address &= ~3;

  if (address >= 0x4000) {
    return ReadUnused(address) >> shift;
  }

  if (state.r15 >= 0x4000) {
    return memory.bios_latch >> shift;
  }

  memory.bios_latch = Read<std::uint32_t>(memory.bios, address);

  return memory.bios_latch >> shift;
}

inline std::uint32_t CPU::ReadUnused(std::uint32_t address) {
  std::uint32_t result = 0;

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
        /* FIXME: this is not correct, but also [$+6] has not been prefetched at this point. */
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
auto CPU::Read_(std::uint32_t address, Access access) -> T {
  int cycles;
  int page = address >> 24;

  if (page != REGION_SRAM_1 && page != REGION_SRAM_2) {
    address &= ~(sizeof(T) - 1);
  }

  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  if (std::is_same_v<T, std::uint32_t>) {
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
      if constexpr (std::is_same_v<T, std::uint32_t>) {
        return (ReadMMIO(address + 0) <<  0) |
               (ReadMMIO(address + 1) <<  8) |
               (ReadMMIO(address + 2) << 16) |
               (ReadMMIO(address + 3) << 24);
      }
      if constexpr (std::is_same_v<T, std::uint16_t>) {
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
    case REGION_ROM_W2_H: {
      // 0x0DXXXXXX may be used to access EEPROM backup.
      if (std::is_same_v<T, std::uint16_t> && IsEEPROMAccess(address)) {
        PrefetchStepROM(address, cycles);
        // TODO: this works in most cases but is not correct.
        if (!dma.IsRunning()) {
          return 1;
        }
        return memory.rom.backup_eeprom->Read(address);
      }
      [[fallthrough]];
    }
    case REGION_ROM_W0_L:
    case REGION_ROM_W0_H:
    case REGION_ROM_W1_L:
    case REGION_ROM_W1_H:
    case REGION_ROM_W2_L: {
      PrefetchStepROM(address, cycles);
      address &= memory.rom.mask;
      if (IsGPIOAccess(address) && memory.rom.gpio->IsReadable()) {
        // TODO: verify 8-bit and 16-bit read behavior.
        if constexpr (std::is_same_v<T, std::uint32_t>) {
          auto lsw = memory.rom.gpio->Read(address + 0);
          auto msw = memory.rom.gpio->Read(address + 2);
          return (msw << 16) | lsw;
        }
        return memory.rom.gpio->Read(address);
      }
      if (address >= memory.rom.size) {
        auto value = address >> 1;
        if constexpr (std::is_same_v<T, std::uint32_t>) {
          return (value & 0xFFFF) | ((value + 1) << 16);
        }
        if constexpr (std::is_same_v<T, std::uint16_t>) {
          return value;
        }
        return value >> ((address & 1) * 8);
      }
      return Read<T>(memory.rom.data.get(), address);
    }
    case REGION_SRAM_1:
    case REGION_SRAM_2: {
      PrefetchStepROM(address, cycles);
      address &= 0x0EFFFFFF;
      std::uint32_t value = 0xFF;
      if (memory.rom.backup_sram) {
        value = memory.rom.backup_sram->Read(address);
      }
      if (std::is_same_v<T, std::uint16_t>) value *= 0x0101;
      if (std::is_same_v<T, std::uint32_t>) value *= 0x01010101;
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
void CPU::Write_(std::uint32_t address, T value, Access access) {
  int cycles;
  int page = address >> 24;

  if (page != REGION_SRAM_1 && page != REGION_SRAM_2) {
    address &= ~(sizeof(T) - 1);
  }

  if ((address & 0x1FFFF) == 0) {
    access = Access::Nonsequential;
  }
  
  if (std::is_same_v<T, std::uint32_t>) {
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
      if constexpr (std::is_same_v<T, std::uint32_t>) {
        WriteMMIO16(address + 0, value & 0xFFFF);
        WriteMMIO16(address + 2, value >> 16);
      }
      if constexpr (std::is_same_v<T, std::uint16_t>) {
        WriteMMIO16(address, value);
      }
      if constexpr (std::is_same_v<T, std::uint8_t>) {
        WriteMMIO(address, value & 0xFF);
      }
      break;
    }
    case REGION_PRAM: {
      PrefetchStepRAM(cycles);
      if constexpr (std::is_same_v<T, std::uint8_t>) {
        Write<std::uint16_t>(ppu.pram, address & 0x3FE, value * 0x0101);
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
      if (std::is_same_v<T, std::uint8_t>) {
        // TODO: move logic to decide the writeable area to the PPU class.
        auto limit = ppu.mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;

        if (address < limit) {
          Write<std::uint16_t>(ppu.vram, address & ~1, value * 0x0101);
        }
      } else {
        Write<T>(ppu.vram, address, value);
      }
      break;
    }
    case REGION_OAM: {
      PrefetchStepRAM(cycles);
      if constexpr (!std::is_same_v<T, std::uint8_t>) {
        Write<T>(ppu.oam, address & 0x3FF, value);
      }
      break;
    }
    case REGION_ROM_W0_L: case REGION_ROM_W0_H:
    case REGION_ROM_W1_L: case REGION_ROM_W1_H:
    case REGION_ROM_W2_L: case REGION_ROM_W2_H: {
      PrefetchStepROM(address, cycles);
      if (std::is_same_v<T, std::uint16_t> && page == REGION_ROM_W2_H && IsEEPROMAccess(address)) {
        // TODO: this probably isn't accurate at all.
        if (dma.IsRunning()) {
          memory.rom.backup_eeprom->Write(address, value);
        }
        return;
      }
      address &= 0x1FFFFFF;
      if (IsGPIOAccess(address)) {
        if constexpr (std::is_same_v<T, std::uint32_t>) {
          memory.rom.gpio->Write(address + 0, (value >>  0) & 0xFF);
          memory.rom.gpio->Write(address + 2, (value >> 16) & 0xFF);
        }
        if constexpr (std::is_same_v<T, std::uint16_t>) {
          memory.rom.gpio->Write(address + 0, value & 0xFF);
          memory.rom.gpio->Write(address + 1, value >> 8);
        }
      }
      break;
    }
    case REGION_SRAM_1:
    case REGION_SRAM_2: {
      PrefetchStepROM(address, cycles);
      address &= 0x0EFFFFFF;
      if (memory.rom.backup_sram) {
        // TODO: why is it seemingly aware of alignment, when the address
        // should by force-aligned by the CPU?
        if (std::is_same_v<T, std::uint32_t>) value >>= (address & 3) * 8;
        if (std::is_same_v<T, std::uint16_t>) value >>= (address & 1) * 8;

        memory.rom.backup_sram->Write(address, value);
      }
      break;
    }
    default: {
      PrefetchStepRAM(cycles);
    }
  }
}
