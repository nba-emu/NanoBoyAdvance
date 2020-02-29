/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <emulator/cartridge/backup/backup.hpp>
#include <emulator/config/config.hpp>
#include <memory>

#include "arm/arm7tdmi.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma.hpp"
#include "hw/timer.hpp"
#include "interrupt.hpp"
#include "scheduler.hpp"

namespace nba::core {

class CPU final : private arm::ARM7TDMI,
                  private arm::MemoryBase {
public:
  using Access = arm::MemoryBase::Access;

  CPU(std::shared_ptr<Config> config);

  void Reset();
  void RunFor(int cycles);
  
  enum MemoryRegion {
    REGION_BIOS  = 0,
    REGION_EWRAM = 2,
    REGION_IWRAM = 3,
    REGION_MMIO  = 4,
    REGION_PRAM  = 5,
    REGION_VRAM  = 6,
    REGION_OAM   = 7,
    REGION_ROM_W0_L = 8,
    REGION_ROM_W0_H = 9,
    REGION_ROM_W1_L = 0xA,
    REGION_ROM_W1_H = 0xB,
    REGION_ROM_W2_L = 0xC,
    REGION_ROM_W2_H = 0xD,
    REGION_SRAM_1 = 0xE,
    REGION_SRAM_2 = 0xF
  };
  
  enum class HaltControl {
    RUN,
    STOP,
    HALT
  };

  std::shared_ptr<Config> config;

  struct SystemMemory {
    std::uint8_t bios[0x04000];
    std::uint8_t wram[0x40000];
    std::uint8_t iram[0x08000];

    struct ROM {
      std::unique_ptr<uint8_t[]> data;
      size_t size;
      std::uint32_t mask = 0x1FFFFFF;

      std::unique_ptr<nba::Backup> backup;
      Config::BackupType backup_type = Config::BackupType::Detect;
    } rom;

    /* Last opcode fetched from BIOS memory. */
    std::uint32_t bios_opcode;
  } memory;

  struct MMIO {
    std::uint16_t keyinput;

    HaltControl haltcnt;

    struct WaitstateControl {
      int sram;
      int ws0_n;
      int ws0_s;
      int ws1_n;
      int ws1_s;
      int ws2_n;
      int ws2_s;
      int phi;
      int prefetch;
      int cgb;
    } waitcnt;

    std::uint16_t rcnt_hack;
  } mmio; 
  
  Scheduler scheduler;
  InterruptController irq_controller;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  
private:
  friend class DMA;
  friend class arm::ARM7TDMI;

  template <typename T>
  auto Read(void* buffer, std::uint32_t address) -> T {
    return *(T*)(&((std::uint8_t*)buffer)[address]);
  }
  
  template <typename T>
  void Write(void* buffer, std::uint32_t address, T value) {
    *(T*)(&((std::uint8_t*)buffer)[address]) = value;
  }

  bool HasEEPROMBackup() const {
    return memory.rom.backup_type == Config::BackupType::EEPROM_4 ||
           memory.rom.backup_type == Config::BackupType::EEPROM_64;
  }
  
  bool IsEEPROMAddress(std::uint32_t address) {
    return HasEEPROMBackup() && ((~memory.rom.size & 0x02000000) || address >= 0x0DFFFF00);
  }

  bool IsROMAddress(std::uint32_t address) {
    return address >= 0x08000000 && address <= 0x0EFFFFFF;
  }

  auto ReadMMIO (std::uint32_t address) -> std::uint8_t;
  void WriteMMIO(std::uint32_t address, std::uint8_t value);
  auto ReadBIOS(std::uint32_t address) -> std::uint32_t;
  auto ReadUnused(std::uint32_t address) -> std::uint32_t;
  
  auto ReadByte(std::uint32_t address, Access access) -> std::uint8_t  final;
  auto ReadHalf(std::uint32_t address, Access access) -> std::uint16_t final;
  auto ReadWord(std::uint32_t address, Access access) -> std::uint32_t final;
  void WriteByte(std::uint32_t address, std::uint8_t value, Access access)  final;
  void WriteHalf(std::uint32_t address, std::uint16_t value, Access access) final;
  void WriteWord(std::uint32_t address, std::uint32_t value, Access access) final;
  
  void Tick(int cycles);
  void Idle() final;
  void PrefetchStep(std::uint32_t address, int cycles);
  
  void UpdateMemoryDelayTable();
  
  /* GamePak prefetch buffer state. */
  struct Prefetch {
    bool active;
    std::uint32_t address[8];
    std::uint32_t last_address;
    int rd_pos;
    int wr_pos;
    int count;
    int countdown;
  } prefetch;
  
  /* Last ROM address that was accessed. Used for GamePak prefetch. */
  std::uint32_t last_rom_address;
  
  int cycles16[2][256] {
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
  };
  
  int cycles32[2][256] {
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 }
  };
  
  static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; /* Non-sequential SRAM/WS0/WS1/WS2 */
  static constexpr int s_ws_seq0[2] = { 2, 1 };       /* Sequential WS0 */
  static constexpr int s_ws_seq1[2] = { 4, 1 };       /* Sequential WS1 */
  static constexpr int s_ws_seq2[2] = { 8, 1 };       /* Sequential WS2 */
};

#include "cpu-memory.inl"
  
} // namespace nba::core
