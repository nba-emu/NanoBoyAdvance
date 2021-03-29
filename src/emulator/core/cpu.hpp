/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <common/m4a.hpp>
#include <emulator/cartridge/backup/backup.hpp>
#include <emulator/cartridge/gpio/gpio.hpp>
#include <emulator/config/config.hpp>
#include <memory>

#include "arm/arm7tdmi.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma.hpp"
#include "hw/interrupt.hpp"
#include "hw/serial.hpp"
#include "hw/timer.hpp"
#include "scheduler.hpp"

namespace nba::core {

class CPU final : private arm::ARM7TDMI, private arm::MemoryBase {
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
      std::unique_ptr<nba::GPIO> gpio;
      std::unique_ptr<nba::Backup> backup_sram;
      std::unique_ptr<nba::Backup> backup_eeprom;
    } rom;

    std::uint32_t bios_latch = 0;
  } memory;

  struct MMIO {
    std::uint16_t keyinput = 0x3FF;
    std::uint16_t rcnt_hack = 0;
    std::uint8_t postflg = 0;
    HaltControl haltcnt = HaltControl::RUN;

    struct WaitstateControl {
      int sram = 0;
      int ws0_n = 0;
      int ws0_s = 0;
      int ws1_n = 0;
      int ws1_s = 0;
      int ws2_n = 0;
      int ws2_s = 0;
      int phi = 0;
      int prefetch = 0;
      int cgb = 0;
    } waitcnt;

    struct KeyControl {
      uint16_t input_mask = 0;
      bool interrupt = false;
      bool and_mode = false;
    } keycnt;

  } mmio;

  Scheduler scheduler;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  SerialBus serial_bus;

private:
  template <typename T>
  auto Read(void* buffer, std::uint32_t address) -> T {
    return *reinterpret_cast<T*>(&(reinterpret_cast<std::uint8_t*>(buffer))[address]);
  }

  template <typename T>
  void Write(void* buffer, std::uint32_t address, T value) {
    *reinterpret_cast<T*>(&(reinterpret_cast<std::uint8_t*>(buffer))[address]) = value;
  }

  bool IsGPIOAccess(std::uint32_t address) {
    // NOTE: we do not check if the address lies within ROM, since
    // it is not required in the context. This should be reflected in the name though.
    return memory.rom.gpio && address >= 0xC4 && address <= 0xC8;
  }

  bool IsEEPROMAccess(std::uint32_t address) {
    return memory.rom.backup_eeprom && ((~memory.rom.size & 0x02000000) || address >= 0x0DFFFF00);
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
  void PrefetchStepRAM(int cycles);
  void PrefetchStepROM(std::uint32_t address, int cycles);
  void UpdateMemoryDelayTable();

  void M4ASearchForSampleFreqSet();
  void M4ASampleFreqSetHook();
  void M4AFixupPercussiveChannels();

  void CheckKeypadInterrupt();
  void OnKeyPress();

  M4ASoundInfo* m4a_soundinfo;
  int m4a_original_freq = 0;
  std::uint32_t m4a_setfreq_address = 0;

  /* GamePak prefetch buffer state. */
  struct Prefetch {
    bool active = false;
    bool rom_code_access = false;
    std::uint32_t head_address;
    std::uint32_t last_address;
    int count = 0;
    int capacity = 8;
    int opcode_width = 4;
    int countdown;
    int duty;
  } prefetch;

  bool bus_is_controlled_by_dma;
  bool openbus_from_dma;

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
