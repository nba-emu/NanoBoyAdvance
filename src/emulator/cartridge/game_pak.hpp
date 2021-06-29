/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <common/integer.hpp>
#include <common/compiler.hpp>
#include <memory>
#include <vector>

#include "backup/eeprom.hpp"
#include "backup/flash.hpp"
#include "backup/sram.hpp"
#include "gpio/rtc.hpp"

namespace nba {

// TODO: handle Nseq access resets EEPROM chip?
// TODO: optimize 32-bit accesses?

struct GamePak {
  GamePak() {}

  GamePak(
    std::vector<u8>&& rom,
    std::unique_ptr<Backup>&& backup,
    std::unique_ptr<GPIO>&& gpio,
    u32 rom_mask = 0x01FF'FFFF
  )   : rom(std::move(rom))
      , gpio(std::move(gpio))
      , rom_mask(rom_mask) {
    if (backup != nullptr) {
      if (typeid(*backup.get()) == typeid(EEPROM)) {
        backup_eeprom = std::move(backup);
      
        if (this->rom.size() >= 0x0200'0000) {
          eeprom_mask = 0x01FF'FF00;
        } else {
          eeprom_mask = 0x0100'0000;
        }
      } else {
        backup_sram = std::move(backup);
      }
    }
  }

  GamePak(GamePak const&) = delete;

  GamePak(GamePak&& other) {
    operator=(std::move(other));
  }

  auto operator=(GamePak const&) -> GamePak& = delete;

  auto operator=(GamePak&& other) -> GamePak& {
    std::swap(rom, other.rom);
    std::swap(backup_sram, other.backup_sram);
    std::swap(backup_eeprom, other.backup_eeprom);
    std::swap(gpio, other.gpio);
    std::swap(rom_mask, other.rom_mask);
    std::swap(eeprom_mask, other.eeprom_mask);
    return *this;
  }

  auto ALWAYS_INLINE ReadROM(u32 address) -> u16 {
    address &= 0x01FF'FFFE;

    // TODO: instead of the IsReadable() check, maybe do not enable GPIO by default.
    if (unlikely(IsGPIO(address)) && gpio->IsReadable()) {
      return gpio->Read(address);
    }

    // TODO: make sure this check is optimized away for lower half addresses
    if (unlikely(IsEEPROM(address))) {
      return backup_eeprom->Read(0);
    }

    address &= rom_mask;

    if (unlikely(address >= rom.size())) {
      return u16(address >> 1);
    }

    // TODO: use a common read<u16> helper.
    return *(u16*)(&rom.data()[address]);
  }

  void ALWAYS_INLINE WriteROM(u32 address, u16 value) {
    address &= 0x01FF'FFFE;

    if (IsGPIO(address)) {
      gpio->Write(address, value);
    }

    if (IsEEPROM(address)) {
      backup_eeprom->Write(0, value);
    }
  }

  auto ALWAYS_INLINE ReadSRAM(u32 address) -> u8 {
    if (likely(backup_sram != nullptr)) {
      return backup_sram->Read(address & 0x0EFF'FFFF);
    }
    return 0xFF;
  }

  void ALWAYS_INLINE WriteSRAM(u32 address, u8 value) {
    if (likely(backup_sram != nullptr)) {
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

  u32 rom_mask = 0;
  u32 eeprom_mask = 0;
};

} // namespace nba
