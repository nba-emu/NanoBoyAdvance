/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>
#include <nba/device/audio_device.hpp>
#include <nba/device/input_device.hpp>
#include <nba/device/video_device.hpp>
#include <nba/integer.hpp>
#include <string>

namespace nba {

struct Config {
  bool skip_bios = false;

  enum class BackupType {
    Detect,
    None,
    SRAM,
    FLASH_64,
    FLASH_128,
    EEPROM_4,
    EEPROM_64
  };

  struct Audio {
    enum class Interpolation {
      Cosine,
      Cubic,
      Sinc_32,
      Sinc_64,
      Sinc_128,
      Sinc_256
    } interpolation = Interpolation::Cosine;

    bool interpolate_fifo = true;
    bool mp2k_hle_enable = false;
    bool mp2k_hle_cubic = false;
  } audio;

  std::shared_ptr<AudioDevice> audio_dev = std::make_shared<NullAudioDevice>();
  std::shared_ptr<InputDevice> input_dev = std::make_shared<NullInputDevice>();
  std::shared_ptr<VideoDevice> video_dev = std::make_shared<NullVideoDevice>();
};

} // namespace nba

namespace std {

using BackupType = nba::Config::BackupType;

inline auto to_string(BackupType value) -> std::string {
  switch (value) {
    case BackupType::Detect: return "Detect";
    case BackupType::None: return "None";
    case BackupType::SRAM: return "SRAM";
    case BackupType::FLASH_64: return "FLASH_64";
    case BackupType::FLASH_128: return "FLASH_128";
    case BackupType::EEPROM_4: return "EEPROM_4";
    default: return "EEPROM_64";
  }
}

} // namespace std
