// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <nba/common/crc32.hh>
#include <nba/rom/gpio/rtc.hh>
#include <nba/rom/gpio/solar_sensor.hh>
#include <atom/logger/logger.hh>

#if defined(PLATFORM_DREAMCAST)
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>
#endif

#include "core.hh"

namespace nba {

namespace core {

Core::Core(std::shared_ptr<Config> config)
    : config(config)
    , cpu(scheduler, bus)
    , irq(cpu, scheduler)
    , dma(bus, irq, scheduler)
    , apu(scheduler, dma, bus, config)
    , ppu(scheduler, irq, dma, config)
    , timer(scheduler, irq, apu)
    , keypad(scheduler, irq)
    , bus(scheduler, {cpu, irq, dma, apu, ppu, timer, keypad}) {
  Reset();
}

void Core::Reset() {
  scheduler.Reset();
  cpu.Reset();
  irq.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  bus.Reset();
  keypad.Reset();

  if(config->skip_bios) {
    SkipBootScreen();
  }

  if(config->audio.mp2k_hle_enable) {
    apu.GetMP2K().UseCubicFilter() = config->audio.mp2k_hle_cubic;
    apu.GetMP2K().ForceReverb() = config->audio.mp2k_hle_force_reverb;
    hle_audio_hook = SearchSoundMainRAM();
    if(hle_audio_hook != 0xFFFFFFFF) {
      ATOM_INFO("Core: detected MP2K audio mixer @ 0x{:08X}", hle_audio_hook);
    }
  } else {
    hle_audio_hook = 0xFFFFFFFF;
  }
}

void Core::Attach(std::vector<u8> const& bios) {
  bus.Attach(bios);
}

void Core::Attach(ROM&& rom) {
  bus.Attach(std::move(rom));
}

auto Core::CreateRTC() -> std::unique_ptr<RTC> {
  return std::make_unique<RTC>(irq);
}

auto Core::CreateSolarSensor() -> std::unique_ptr<SolarSensor> {
  return std::make_unique<SolarSensor>();
}

void Core::SetKeyStatus(Key key, bool pressed) {
  keypad.SetKeyStatus(key, pressed);
}

void Core::Run(int cycles) {
  using HaltControl = Bus::Hardware::HaltControl;

  const auto limit = scheduler.GetTimestampNow() + cycles;

  while(scheduler.GetTimestampNow() < limit) {
    if(bus.hw.haltcnt == HaltControl::Run) {
      if(cpu.state.r15 == hle_audio_hook) {
        if(auto* sound_info_ptr = bus.GetHostAddress<u32>(0x03007FF0)) {
          const u32 sound_info_addr = *sound_info_ptr;
          if(const auto sound_info = bus.GetHostAddress<MP2K::SoundInfo>(sound_info_addr)) {
            apu.GetMP2K().SoundMainRAM(*sound_info);
          }
        }
      }

      cpu.Run();
    } else {
      while(scheduler.GetTimestampNow() < limit && !irq.ShouldUnhaltCPU()) {
        if(dma.IsRunning()) {
          dma.Run();
          if(irq.ShouldUnhaltCPU()) continue; // can become true during the DMA
        }

        bus.Step(scheduler.GetRemainingCycleCount());
      }

      if(irq.ShouldUnhaltCPU()) {
        bus.Step(1);
        bus.hw.haltcnt = HaltControl::Run;
      }
    }
  }
}

void Core::SkipBootScreen() {
  cpu.SwitchMode(arm::MODE_SYS);
  cpu.state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
  cpu.state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
  cpu.state.r13 = 0x03007F00;
  cpu.state.r15 = 0x08000000;
}

auto Core::SearchSoundMainRAM() -> u32 {
  static constexpr u32 kSoundMainCRC32 = 0x27EA7FCF;
  static constexpr int kSoundMainLength = 48;

  auto& rom_vec = bus.memory.rom.GetRawROM();

  if(rom_vec.size() >= kSoundMainLength) {
    u32 address_max = rom_vec.size() - kSoundMainLength;

    for(u32 address = 0; address <= address_max; address += sizeof(u16)) {
      auto crc = crc32(&rom_vec[address], kSoundMainLength);

      if(crc == kSoundMainCRC32) {
        /* We have found SoundMain().
         * The pointer to SoundMainRAM() is stored at offset 0x74.
         */
        address = read<u32>(rom_vec.data(), address + 0x74);
        if(address & 1) {
          address &= ~1;
          address += sizeof(u16) * 2;
        } else {
          address &= ~3;
          address += sizeof(u32) * 2;
        }
        return address;
      }
    }

    return 0xFFFFFFFF;
  }

#if defined(PLATFORM_DREAMCAST)
  return SearchSoundMainRAMFromFile();
#else
  return 0xFFFFFFFF;
#endif
}

#if defined(PLATFORM_DREAMCAST)
auto Core::SearchSoundMainRAMFromFile() -> u32 {
  static constexpr u32 kSoundMainCRC32 = 0x27EA7FCF;
  static constexpr int kSoundMainLength = 48;

  // After finding SoundMain, we read a pointer at offset 0x74 (116 bytes).
  // The overlap past kChunkSize must cover both the 48-byte CRC window AND
  // the 4-byte pointer read at offset 0x74, giving 0x74 + 4 = 120 bytes.
  static constexpr size_t kChunkSize   = 64 * 1024;
  static constexpr size_t kOverlapSize = 0x74 + sizeof(u32); // 120 bytes

  // MP2K SoundMain lives in the game-engine portion of ROM.  Capping the
  // search at 8 MiB covers virtually all GBA titles while bounding the CD
  // read time to a few seconds instead of scanning the full 16+ MiB.
  static constexpr size_t kMaxSearchSize = 8 * 1024 * 1024;

  auto& rom = bus.memory.rom;
  if(!rom.IsPagedROM()) {
    return 0xFFFFFFFF;
  }

  const size_t rom_size   = std::min(rom.GetPagedROMSize(), kMaxSearchSize);
  const auto& rom_path    = rom.GetPagedROMPath();

  auto* file = std::fopen(rom_path.c_str(), "rb");
  if(!file) {
    return 0xFFFFFFFF;
  }

  std::vector<u8> chunk(kChunkSize + kOverlapSize);
  u32 result = 0xFFFFFFFF;
  size_t file_offset = 0;

  while(file_offset < rom_size && result == 0xFFFFFFFF) {
    const size_t to_read = std::min(kChunkSize + kOverlapSize, rom_size - file_offset);
    if(std::fseek(file, static_cast<long>(file_offset), SEEK_SET) != 0) break;
    const size_t bytes_read = std::fread(chunk.data(), 1, to_read, file);
    if(bytes_read < static_cast<size_t>(kSoundMainLength)) break;

    // Only scan positions where the full 48-byte CRC window AND the 120-byte
    // pointer read both fit within the buffer we actually read.
    const size_t safe_limit = (bytes_read >= kOverlapSize)
      ? std::min(bytes_read - kOverlapSize, kChunkSize)
      : 0;

    for(size_t i = 0; i <= safe_limit; i += sizeof(u16)) {
      if(crc32(&chunk[i], kSoundMainLength) == kSoundMainCRC32) {
        u32 address;
        std::memcpy(&address, &chunk[i + 0x74], sizeof(u32));
        if(address & 1) {
          address &= ~1u;
          address += sizeof(u16) * 2;
        } else {
          address &= ~3u;
          address += sizeof(u32) * 2;
        }
        result = address;
        break;
      }
    }

    file_offset += kChunkSize;
  }

  std::fclose(file);
  return result;
}
#endif

auto Core::GetROM() -> ROM& {
  return bus.memory.rom;
}

auto Core::GetPRAM() -> u8* {
  return ppu.GetPRAM();
}

auto Core::GetVRAM() -> u8* {
  return ppu.GetVRAM();
}

auto Core::GetOAM() -> u8* {
  return ppu.GetOAM();
}

auto Core::PeekByteIO(u32 address) -> u8  {
  return bus.hw.ReadByte(address);
}

auto Core::PeekHalfIO(u32 address) -> u16 {
  return bus.hw.ReadHalf(address);
}

auto Core::PeekWordIO(u32 address) -> u32 {
  return bus.hw.ReadWord(address);
}

auto Core::GetBGHOFS(int id) -> u16 {
  return ppu.mmio.bghofs[id];
}

auto Core::GetBGVOFS(int id) -> u16 {
  return ppu.mmio.bgvofs[id];
}

Scheduler& Core::GetScheduler() {
  return scheduler;
}

} // namespace nba::core

auto CreateCore(
  std::shared_ptr<Config> config
) -> std::unique_ptr<CoreBase> {
  return std::make_unique<core::Core>(config);
}

} // namespace nba
