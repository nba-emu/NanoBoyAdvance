/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <memory>
#include <nba/integer.hpp>
#include <nba/rom/backup/eeprom.hpp>
#include <nba/rom/gpio/gpio.hpp>
#include <nba/common/compiler.hpp>
#include <nba/common/punning.hpp>
#include <nba/save_state.hpp>
#include <vector>

namespace nba {

/**
 * TODO:
 *  - emulate the EEPROM being selected and deselected at the start/end of burst transfers?
 *  - optimize EEPROM check away for lower-half ROM address space
 */

struct ROM {
  ROM() {}

  ROM(
    std::vector<u8>&& rom,
    std::unique_ptr<Backup>&& backup,
    std::unique_ptr<GPIO>&& gpio,
    u32 rom_mask = 0x01FF'FFFF
  )   : rom(std::move(rom))
      , gpio(std::move(gpio))
      , rom_mask(rom_mask) {
    if(backup != nullptr) {
      if(typeid(*backup.get()) == typeid(EEPROM)) {
        backup_eeprom = std::move(backup);

        if(this->rom.size() >= 0x0100'0001) {
          eeprom_mask = 0x01FF'FF00;
        } else {
          eeprom_mask = 0x0100'0000;
        }
      } else {
        backup_sram = std::move(backup);
      }
    }
  }

  ROM(ROM const&) = delete;

  ROM(ROM&& other) {
    operator=(std::move(other));
  }

  auto operator=(ROM const&) -> ROM& = delete;

  auto operator=(ROM&& other) -> ROM& {
    std::swap(rom, other.rom);
    std::swap(backup_sram, other.backup_sram);
    std::swap(backup_eeprom, other.backup_eeprom);
    std::swap(gpio, other.gpio);
    std::swap(rom_mask, other.rom_mask);
    std::swap(eeprom_mask, other.eeprom_mask);
    return *this;
  }

  auto GetRawROM() -> std::vector<u8>& {
    return rom;
  }

  template<typename T>
  auto GetGPIODevice() -> T* {
    if(gpio) {
      return gpio->Get<T>();
    }
    return nullptr;
  }

  void LoadState(SaveState const& state) {
    rom_address_latch = state.rom_address_latch;

    if(backup_sram) {
      backup_sram->LoadState(state);
    }

    if(backup_eeprom) {
      backup_eeprom->LoadState(state);
    }

    if(gpio) {
      gpio->LoadState(state);
    }
  }

  void CopyState(SaveState& state) {
    state.rom_address_latch = rom_address_latch;

    if(backup_sram) {
      backup_sram->CopyState(state);
    }

    if(backup_eeprom) {
      backup_eeprom->CopyState(state);
    }

    if(gpio) {
      gpio->CopyState(state);
    }
  }

  void SetEEPROMSizeHint(EEPROM::Size size) {
    if(backup_eeprom) {
      ((EEPROM*)backup_eeprom.get())->SetSizeHint(size);
    }
  }

  auto ALWAYS_INLINE ReadROM16(u32 address, bool sequential) -> u16 {
    address &= 0x01FF'FFFE;

    if(unlikely(IsGPIO(address)) && gpio->IsReadable()) {
      return gpio->Read(address);
    }

    if(unlikely(IsEEPROM(address))) {
      return backup_eeprom->Read(0);
    }

    u16 data;

    if(!sequential) {
      // According to Reiner Ziegler official GBA cartridges latch A0 to A23 (not just A0 to A15)
      // "In theory, you can read an entire GBA ROM with just one non-sequential read (address 0) and all of the other
      //  reads as sequential so address counters must be used on most address lines to exactly emulate a GBA ROM.
      //  However, you only need to use address latch / counters on A0-A15 in order to satisfy the GBA since A16-A23 are always accurate."
      // Source: https://reinerziegler.de.mirrors.gg8.se/GBA/gba.htm#GBA%20cartridges
      rom_address_latch = address & rom_mask;
    }

    if(likely(rom_address_latch < rom.size())) {
      data = read<u16>(rom.data(), rom_address_latch);
    } else {
      data = (u16)(rom_address_latch >> 1);
    }

    rom_address_latch = (rom_address_latch + sizeof(u16)) & rom_mask;
    return data;
  }

  auto ALWAYS_INLINE ReadROM32(u32 address, bool sequential) -> u32 {
    address &= 0x01FF'FFFC;

    if(unlikely(IsGPIO(address)) && gpio->IsReadable()) {
      auto lsw = gpio->Read(address|0);
      auto msw = gpio->Read(address|2);
      return (msw << 16) | lsw;
    }

    if(unlikely(IsEEPROM(address))) {
      auto lsw = backup_eeprom->Read(0);
      auto msw = backup_eeprom->Read(0);
      return (msw << 16) | lsw;
    }

    u32 data;

    if(!sequential) {
      rom_address_latch = address & rom_mask;
    }

    if(likely(rom_address_latch < rom.size())) {
      data = read<u32>(rom.data(), rom_address_latch);
    } else {
      const u16 lsw = (u16)(rom_address_latch >> 1);
      const u16 msw = (u16)(lsw + 1);

      data = (msw << 16) | lsw;
    }

    rom_address_latch = (rom_address_latch + sizeof(u32)) & rom_mask;
    return data;
  }

  void ALWAYS_INLINE WriteROM(u32 address, u16 value, bool sequential) {
    address &= 0x01FF'FFFE;

    if(IsGPIO(address)) {
      gpio->Write(address, value);
    }

    if(IsEEPROM(address)) {
      backup_eeprom->Write(0, value);
    } else if(!sequential) {
      // @todo: confirm that a) EEPROM accesses do not update the latched address (or that it doesn't matter) and that b) writes do update the latch.
      rom_address_latch = address & rom_mask;
    }
  }

  auto ALWAYS_INLINE ReadSRAM(u32 address) -> u8 {
    if(likely(backup_sram != nullptr)) {
      return backup_sram->Read(address & 0x0EFF'FFFF);
    }
    return 0xFF;
  }

  void ALWAYS_INLINE WriteSRAM(u32 address, u8 value) {
    if(likely(backup_sram != nullptr)) {
      backup_sram->Write(address & 0x0EFF'FFFF, value);
    }
  }

private:
  bool ALWAYS_INLINE IsGPIO(u32 address) {
    return gpio && address >= 0xC4 && address <= 0xC8;
  }

  bool ALWAYS_INLINE IsEEPROM(u32 address) {
    return backup_eeprom && (address & eeprom_mask) == eeprom_mask;
  }

  std::vector<u8> rom;
  std::unique_ptr<Backup> backup_sram;
  std::unique_ptr<Backup> backup_eeprom;
  std::unique_ptr<GPIO> gpio;

  u32 rom_address_latch = 0;
  u32 rom_mask = 0;
  u32 eeprom_mask = 0;
};

} // namespace nba
