// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/common/compiler.hh>
#include <nba/common/punning.hh>
#include <nba/rom/backup/eeprom.hh>
#include <nba/rom/gpio/gpio.hh>
#include <nba/integer.hh>
#include <nba/save_state.hh>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace nba {

/**
 * TODO:
 *  - emulate the EEPROM being selected and deselected at the start/end of burst transfers?
 *  - optimize EEPROM check away for lower-half ROM address space
 */

struct ROM {
  static constexpr size_t kPageSize = 1024 * 1024;
  static constexpr size_t kPageCount = 4;
  static constexpr size_t kSmallROMPageCount = 2;
  static constexpr size_t kLargeROMThreshold = 8 * 1024 * 1024;

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

#if defined(PLATFORM_DREAMCAST)
  ROM(
    std::string&& path,
    size_t size,
    std::unique_ptr<Backup>&& backup,
    std::unique_ptr<GPIO>&& gpio,
    u32 rom_mask = 0x01FF'FFFF
  )   : rom_path(std::move(path))
      , rom_size(size)
      , backup_sram(nullptr)
      , backup_eeprom(nullptr)
      , gpio(std::move(gpio))
      , rom_mask(rom_mask) {
    rom_file = std::fopen(rom_path.c_str(), "rb");

    if(backup != nullptr) {
      if(typeid(*backup.get()) == typeid(EEPROM)) {
        backup_eeprom = std::move(backup);

        if(rom_size >= 0x0100'0001) {
          eeprom_mask = 0x01FF'FF00;
        } else {
          eeprom_mask = 0x0100'0000;
        }
      } else {
        backup_sram = std::move(backup);
      }
    }
  }
#endif

  ~ROM() {
#if defined(PLATFORM_DREAMCAST)
    ClosePagedFile();
#endif
  }

  ROM(ROM const&) = delete;

  ROM(ROM&& other) {
    operator=(std::move(other));
  }

  auto operator=(ROM const&) -> ROM& = delete;

  auto operator=(ROM&& other) -> ROM& {
#if defined(PLATFORM_DREAMCAST)
    ClosePagedFile();
#endif
    std::swap(rom, other.rom);
#if defined(PLATFORM_DREAMCAST)
    std::swap(rom_path, other.rom_path);
    std::swap(rom_size, other.rom_size);
    std::swap(rom_file, other.rom_file);
    std::swap(rom_pages, other.rom_pages);
    std::swap(rom_page_clock, other.rom_page_clock);
    std::swap(rom_read_error, other.rom_read_error);
    std::swap(rom_page_miss_count, other.rom_page_miss_count);
#endif
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

  auto CopyRange(u32 address, size_t size, u8* dest) -> bool {
    if(dest == nullptr) {
      return size == 0;
    }

    if(size == 0) {
      return true;
    }

    if(!rom.empty()) {
      if(address > rom.size() || size > rom.size() - address) {
        return false;
      }

      std::memcpy(dest, rom.data() + address, size);
      return true;
    }

#if defined(PLATFORM_DREAMCAST)
    if(address > rom_size || size > rom_size - address) {
      return false;
    }

    if(!IsPagedROM()) {
      return false;
    }

    size_t copied = 0;
    while(copied < size) {
      const u32 addr = address + static_cast<u32>(copied);
      auto* page = LoadPagedROM(addr);
      if(page == nullptr) {
        return false;
      }

      const auto page_offset = addr & (u32(kPageSize) - 1);
      const size_t page_remaining = kPageSize - static_cast<size_t>(page_offset);
      const size_t chunk = std::min(size - copied, page_remaining);

      std::memcpy(dest + copied, page->data.data() + page_offset, chunk);
      copied += chunk;
    }

    return true;
#else
    return false;
#endif
  }

#if defined(PLATFORM_DREAMCAST)
  auto IsPagedROM() const -> bool {
    return rom_file != nullptr;
  }

  auto GetPagedROMSize() const -> size_t {
    return rom_size;
  }

  auto GetPagedROMPath() const -> std::string const& {
    return rom_path;
  }

  // Warm the page cache with the first page of the ROM so that the initial
  // burst of CPU instruction fetches after Reset() hits the cache rather than
  // triggering CD-ROM I/O on the first executed opcode.
  auto PreloadFirstPage() -> bool {
    if(!IsPagedROM()) {
      return true;
    }

    rom_read_error = false;
    return LoadPagedROM(0) != nullptr;
  }

  auto HasReadError() const -> bool {
    return rom_read_error;
  }

  auto GetActivePageCount() const -> size_t {
    return ActivePageCount();
  }

  auto TakePageMissCount() -> u32 {
    const u32 count = rom_page_miss_count;
    rom_page_miss_count = 0;
    return count;
  }

  // Returns a host pointer when the entire range is resident in a single page
  // cache slot.  The pointer is only valid until the next paged ROM read.
  auto GetHostAddressRange(u32 address, size_t size) -> u8* {
    if(size == 0 || address > rom_size || size > rom_size - address) {
      return nullptr;
    }

    const auto page_start = address & ~(u32(kPageSize) - 1);
    if(size > (page_start + kPageSize) - address) {
      return nullptr;
    }

    auto* page = LoadPagedROM(address);
    if(page == nullptr) {
      return nullptr;
    }

    return page->data.data() + (address & (u32(kPageSize) - 1));
  }
#endif

  template<typename T>
  auto GetGPIODevice() -> T* {
    if(gpio) {
      return gpio->Get<T>();
    }
    return nullptr;
  }

  // Whether this cartridge has a save backup at all.
  auto HasBackup() const -> bool {
    return backup_sram != nullptr || backup_eeprom != nullptr;
  }

  // Whether every present backup is streaming its writes to disk.  Returns
  // false when a backup exists but its save only lives in memory this session
  // (e.g. read-only media), which the frontend can surface to the user.
  auto IsBackupPersistent() const -> bool {
    if(backup_sram && !backup_sram->IsPersistent()) {
      return false;
    }
    if(backup_eeprom && !backup_eeprom->IsPersistent()) {
      return false;
    }
    return true;
  }

  // Best-effort persist of all present backups to disk.  Returns true only if
  // every backup is safely on disk afterwards.
  auto FlushBackup() -> bool {
    bool ok = true;
    if(backup_sram) {
      ok = backup_sram->Flush() && ok;
    }
    if(backup_eeprom) {
      ok = backup_eeprom->Flush() && ok;
    }
    return ok;
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

    if(IsGPIO(address) && gpio->IsReadable()) [[unlikely]] {
      return gpio->Read(address);
    }

    if(IsEEPROM(address)) [[unlikely]] {
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

    if(rom_address_latch <= rom.size() &&
        sizeof(u16) <= rom.size() - rom_address_latch) [[likely]] {
      data = read<u16>(rom.data(), rom_address_latch);
#if defined(PLATFORM_DREAMCAST)
    } else if(IsPagedROM() && rom_address_latch + sizeof(u16) <= rom_size) {
      data = ReadPaged16(rom_address_latch);
#endif
    } else {
      data = (u16)(rom_address_latch >> 1);
    }

    rom_address_latch = (rom_address_latch + sizeof(u16)) & rom_mask;
    return data;
  }

  auto ALWAYS_INLINE ReadROM32(u32 address, bool sequential) -> u32 {
    address &= 0x01FF'FFFC;

    if(IsGPIO(address) && gpio->IsReadable()) [[unlikely]] {
      auto lsw = gpio->Read(address|0);
      auto msw = gpio->Read(address|2);
      return (msw << 16) | lsw;
    }

    if(IsEEPROM(address)) [[unlikely]] {
      auto lsw = backup_eeprom->Read(0);
      auto msw = backup_eeprom->Read(0);
      return (msw << 16) | lsw;
    }

    u32 data;

    if(!sequential) {
      rom_address_latch = address & rom_mask;
    }

    if(rom_address_latch <= rom.size() &&
        sizeof(u32) <= rom.size() - rom_address_latch) [[likely]] {
      data = read<u32>(rom.data(), rom_address_latch);
#if defined(PLATFORM_DREAMCAST)
    } else if(IsPagedROM() && rom_address_latch + sizeof(u32) <= rom_size) {
      data = ReadPaged32(rom_address_latch);
#endif
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
    if(backup_sram != nullptr) [[likely]] {
      return backup_sram->Read(address & 0x0EFF'FFFF);
    }
    return 0xFF;
  }

  void ALWAYS_INLINE WriteSRAM(u32 address, u8 value) {
    if(backup_sram != nullptr) [[likely]] {
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

#if defined(PLATFORM_DREAMCAST)
  struct PagedROMPage {
    std::vector<u8> data;
    u32 start = 0;
    u32 last_used = 0;
    bool valid = false;
  };

  void ClosePagedFile() {
    if(rom_file) {
      std::fclose(rom_file);
      rom_file = nullptr;
    }
  }

  // Resolves the page containing `address`, reading it from the backing file
  // (with LRU eviction) on a cache miss.  Returns the resident page, or nullptr
  // on a media read failure.  Returning the page directly lets callers read
  // straight from the page buffer instead of re-scanning the cache.
  auto LoadPagedROM(u32 address) -> PagedROMPage* {
    if(!rom_file) {
      rom_read_error = true;
      return nullptr;
    }

    const auto page_start = address & ~(u32(kPageSize) - 1);
    const size_t active_pages = ActivePageCount();
    rom_page_clock++;

    for(size_t page_index = 0; page_index < active_pages; page_index++) {
      auto& page = rom_pages[page_index];
      if(page.valid && page.start == page_start) {
        page.last_used = rom_page_clock;
        return rom_read_error ? nullptr : &page;
      }
    }

    auto* target = &rom_pages[0];
    for(size_t page_index = 0; page_index < active_pages; page_index++) {
      auto& page = rom_pages[page_index];
      if(!page.valid) {
        target = &page;
        break;
      }

      if(page.last_used < target->last_used) {
        target = &page;
      }
    }

    rom_page_miss_count++;

    size_t bytes_to_read = kPageSize;
    if(page_start + bytes_to_read > rom_size) {
      bytes_to_read = rom_size - page_start;
    }

    if(target->data.empty()) {
      target->data.resize(kPageSize);
    }

    if(std::fseek(rom_file, static_cast<long>(page_start), SEEK_SET) != 0) {
      rom_read_error = true;
      target->valid = false;
      return nullptr;
    }

    const auto bytes_read = std::fread(target->data.data(), 1, bytes_to_read, rom_file);

    if(bytes_read < bytes_to_read) {
      if(page_start + bytes_to_read >= rom_size) {
        // Final ROM page may be smaller than kPageSize; pad the remainder.
        std::memset(target->data.data() + bytes_read, 0, kPageSize - bytes_read);
      } else {
        rom_read_error = true;
        target->valid = false;
        return nullptr;
      }
    } else if(bytes_read < kPageSize) {
      std::memset(target->data.data() + bytes_read, 0, kPageSize - bytes_read);
    }

    target->start = page_start;
    target->last_used = rom_page_clock;
    target->valid = true;
    return target;
  }

  auto ReadPaged8(u32 address) -> u8 {
    if(address >= rom_size) {
      return 0xFF;
    }
    if(auto* page = LoadPagedROM(address)) {
      return page->data[address & (u32(kPageSize) - 1)];
    }
    return 0xFF;
  }

  auto ReadPaged16(u32 address) -> u16 {
    const auto page_offset = address & (u32(kPageSize) - 1);

    // Fast path: the whole halfword lives inside a single page, so resolve the
    // page once and read it directly instead of fetching byte-by-byte.
    if(page_offset <= kPageSize - sizeof(u16)) [[likely]] {
      if(address >= rom_size) {
        return 0xFFFF;
      }
      if(auto* page = LoadPagedROM(address)) {
        return read<u16>(page->data.data(), page_offset);
      }
      return 0xFFFF;
    }

    // Slow path: the access straddles a page boundary.
    return u16(u32(ReadPaged8(address)) | (u32(ReadPaged8(address + 1)) << 8));
  }

  auto ReadPaged32(u32 address) -> u32 {
    const auto page_offset = address & (u32(kPageSize) - 1);

    // Fast path: the whole word lives inside a single page.
    if(page_offset <= kPageSize - sizeof(u32)) [[likely]] {
      if(address >= rom_size) {
        return 0xFFFF'FFFF;
      }
      if(auto* page = LoadPagedROM(address)) {
        return read<u32>(page->data.data(), page_offset);
      }
      return 0xFFFF'FFFF;
    }

    // Slow path: the access straddles a page boundary.
    return u32(
      u32(ReadPaged8(address)) |
      (u32(ReadPaged8(address + 1)) << 8) |
      (u32(ReadPaged8(address + 2)) << 16) |
      (u32(ReadPaged8(address + 3)) << 24)
    );
  }

  auto ActivePageCount() const -> size_t {
    return rom_size > kLargeROMThreshold ? kPageCount : kSmallROMPageCount;
  }
#endif

  std::vector<u8> rom;
#if defined(PLATFORM_DREAMCAST)
  std::string rom_path;
  size_t rom_size = 0;
  FILE* rom_file = nullptr;
  std::array<PagedROMPage, kPageCount> rom_pages;
  u32 rom_page_clock = 0;
  u32 rom_page_miss_count = 0;
  bool rom_read_error = false;
#endif
  std::unique_ptr<Backup> backup_sram;
  std::unique_ptr<Backup> backup_eeprom;
  std::unique_ptr<GPIO> gpio;

  u32 rom_address_latch = 0;
  u32 rom_mask = 0;
  u32 eeprom_mask = 0;
};

} // namespace nba
