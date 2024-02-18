/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>
#include <nba/rom/backup/eeprom.hpp>
#include <nba/rom/backup/flash.hpp>
#include <nba/rom/backup/sram.hpp>

namespace nba {

void EEPROM::LoadState(SaveState const& state) {
  this->state = state.backup.eeprom.state;
  address = state.backup.eeprom.address;
  serial_buffer = state.backup.eeprom.serial_buffer;
  transmitted_bits = state.backup.eeprom.transmitted_bits;

  std::memcpy(file->Buffer(), state.backup.data, file->Size());
}

void EEPROM::CopyState(SaveState& state) {
  state.backup.eeprom.state = this->state;
  state.backup.eeprom.address = address;
  state.backup.eeprom.serial_buffer = serial_buffer;
  state.backup.eeprom.transmitted_bits = transmitted_bits;

  std::memcpy(state.backup.data, file->Buffer(), file->Size());
}

void FLASH::LoadState(SaveState const& state) {
  current_bank = state.backup.flash.current_bank;
  phase = state.backup.flash.phase;
  enable_chip_id = state.backup.flash.enable_chip_id;
  enable_erase = state.backup.flash.enable_erase;
  enable_write = state.backup.flash.enable_write;
  enable_select = state.backup.flash.enable_select;

  std::memcpy(file->Buffer(), state.backup.data, file->Size());
}

void FLASH::CopyState(SaveState& state) {
  state.backup.flash.current_bank = current_bank;
  state.backup.flash.phase = phase;
  state.backup.flash.enable_chip_id = enable_chip_id;
  state.backup.flash.enable_erase = enable_erase;
  state.backup.flash.enable_write = enable_write;
  state.backup.flash.enable_select = enable_select;

  std::memcpy(state.backup.data, file->Buffer(), file->Size());
}

void SRAM::LoadState(SaveState const& state) {
  std::memcpy(file->Buffer(), state.backup.data, file->Size());
}

void SRAM::CopyState(SaveState& state) {
  std::memcpy(state.backup.data, file->Buffer(), file->Size());
}

} // namespace nba