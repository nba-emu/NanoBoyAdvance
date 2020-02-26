/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba::core {

enum class InterruptSource {
  VBlank,
  HBlank,
  VCount,
  Timer,
  Serial,
  DMA,
  Keypad,
  GamePak
};

class InterruptController {
public:
  InterruptController() { Reset(); }

  void Reset() {
    master_enable = 0;
    enabled = 0;
    requested = 0;
  }

  auto Read(int offset) const -> std::uint8_t {
    switch (offset) {
      case 0: return enabled  & 0xFF;
      case 1: return enabled >> 8;
      case 2: return requested  & 0xFF;
      case 3: return requested >> 8;
      case 4: return master_enable ? 1 : 0;
    }

    return 0;
  }

  void Write(int offset, std::uint8_t value) {
    switch (offset) {
      case 0: {
        enabled &= 0xFF00;
        enabled |= value;
        break;
      }
      case 1: {
        enabled &= 0x00FF;
        enabled |= value << 8;
        break;
      }
      case 2: requested &= ~value; break;
      case 3: requested &= ~(value << 8); break;
      case 4: master_enable = value & 1; break;
    }
  }

  bool MasterEnable() const {
    return master_enable != 0;
  }

  bool HasServableIRQ() const {
    return (requested & enabled) != 0;
  }

  void Raise(InterruptSource source, int id = 0) {
    switch (source) {
      case InterruptSource::VBlank: {
        requested |= 1;
        break;
      }
      case InterruptSource::HBlank: {
        requested |= 2;
        break;
      }
      case InterruptSource::VCount: {
        requested |= 4;
        break;
      }
      case InterruptSource::Timer: {
        requested |= 8 << id;
        break;
      }
      case InterruptSource::Serial: {
        requested |= 128;
        break;
      }
      case InterruptSource::DMA: {
        requested |= 256 << id;
        break;
      }
      case InterruptSource::Keypad: {
        requested |= 4096;
        break;
      }
      case InterruptSource::GamePak: {
        requested |= 8192;
        break;
      }
    }
  }

private:
  bool master_enable;
  std::uint16_t enabled;
  std::uint16_t requested;
};

} // namespace nba::core
