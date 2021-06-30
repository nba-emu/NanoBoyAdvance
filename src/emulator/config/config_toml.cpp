/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <toml.hpp>

#include "config_toml.hpp"

namespace nba {

void config_toml_read(Config& config, std::string const& path) {
  if (!std::filesystem::exists(path)) {
    auto default_config = Config{};
    config_toml_write(default_config, path);
    LOG_WARN("No configuration file found, created default configuration file.");
    return;
  }

  toml::value data;

  try {
    data = toml::parse(path);
  } catch (std::exception& ex) {
    LOG_ERROR("Error while parsing TOML configuration for read: {0}", ex.what());
    return;
  }

  if (data.contains("general")) {
    auto general_result = toml::expect<toml::value>(data.at("general"));

    if (general_result.is_ok()) {
      auto general = general_result.unwrap();
      config.bios_path = toml::find_or<std::string>(general, "bios_path", "bios.bin");
      config.skip_bios = toml::find_or<toml::boolean>(general, "bios_skip", false);
      config.sync_to_audio = toml::find_or<toml::boolean>(general, "sync_to_audio", true);
    }
  }

  if (data.contains("cartridge")) {
    auto cartridge_result = toml::expect<toml::value>(data.at("cartridge"));

    if (cartridge_result.is_ok()) {
      auto cartridge = cartridge_result.unwrap();
      auto save_type = toml::find_or<std::string>(cartridge, "save_type", "detect");

      const std::map<std::string, Config::BackupType> save_types{
        { "detect",     Config::BackupType::Detect    },
        { "none",       Config::BackupType::None      },
        { "sram",       Config::BackupType::SRAM      },
        { "flash64",    Config::BackupType::FLASH_64  },
        { "flash128",   Config::BackupType::FLASH_128 },
        { "eeprom512",  Config::BackupType::EEPROM_4  },
        { "eeprom8192", Config::BackupType::EEPROM_64 }
      };

      auto match = save_types.find(save_type);

      if (match == save_types.end()) {
        LOG_WARN("Save type '{0}' is not valid, defaulting to auto-detect.", save_type);
        config.backup_type = Config::BackupType::Detect;
      } else {
        config.backup_type = match->second;
      }

      config.force_rtc = toml::find_or<toml::boolean>(cartridge, "force_rtc", false);
    }
  }

  if (data.contains("video")) {
    auto video_result = toml::expect<toml::value>(data.at("video"));

    if (video_result.is_ok()) {
      auto video = video_result.unwrap();
      config.video.fullscreen = toml::find_or<toml::boolean>(video, "fullscreen", false);
      config.video.scale = toml::find_or<int>(video, "scale", 2);
      config.video.shader.path_vs = toml::find_or<std::string>(video, "shader_vs", "");
      config.video.shader.path_fs = toml::find_or<std::string>(video, "shader_fs", "");
    }
  }

  if (data.contains("audio")) {
    auto audio_result = toml::expect<toml::value>(data.at("audio"));

    if (audio_result.is_ok()) {
      auto audio = audio_result.unwrap();
      auto resampler = toml::find_or<std::string>(audio, "resampler", "cosine");

      const std::map<std::string, Config::Audio::Interpolation> resamplers{
        { "cosine",  Config::Audio::Interpolation::Cosine   },
        { "cubic",   Config::Audio::Interpolation::Cubic    },
        { "sinc64",  Config::Audio::Interpolation::Sinc_64  },
        { "sinc128", Config::Audio::Interpolation::Sinc_128 },
        { "sinc256", Config::Audio::Interpolation::Sinc_256 }
      };

      auto match = resamplers.find(resampler);

      if (match == resamplers.end()) {
        LOG_WARN("Resampler '{0}' is not valid, defaulting to cosine resampler.", resampler);
        config.audio.interpolation = Config::Audio::Interpolation::Cosine;
      } else {
        config.audio.interpolation = match->second;
      }

      config.audio.interpolate_fifo = toml::find_or<toml::boolean>(audio, "interpolate_fifo", true);
      config.audio.m4a_xq_enable = toml::find_or<toml::boolean>(audio, "m4a_xq_enable", false);
    }
  }
}

void config_toml_write(Config& config, std::string const& path) {
  toml::basic_value<toml::preserve_comments> data;

  if (std::filesystem::exists(path)) {
    try {
      data = toml::parse<toml::preserve_comments>(path);
    } catch (std::exception& ex) {
      LOG_ERROR("Error while parsing TOML configuration for update: {0}\n\nThe file was not updated.", ex.what());
      return;
    }
  }

  // General
  data["general"]["bios_path"] = config.bios_path;
  data["general"]["bios_skip"] = config.skip_bios;
  data["general"]["sync_to_audio"] = config.sync_to_audio;

  // Cartridge
  std::string save_type;
  switch (config.backup_type) {
    case Config::BackupType::Detect: save_type = "detect"; break;
    case Config::BackupType::None:   save_type = "none"; break;
    case Config::BackupType::SRAM:   save_type = "sram"; break;
    case Config::BackupType::FLASH_64:  save_type = "flash64"; break;
    case Config::BackupType::FLASH_128: save_type = "flash128"; break;
    case Config::BackupType::EEPROM_4:  save_type = "eeprom512"; break;
    case Config::BackupType::EEPROM_64: save_type = "eeprom8192"; break;
  }
  data["cartridge"]["save_type"] = save_type;
  data["cartridge"]["force_rtc"] = config.force_rtc;

  // Video
  data["video"]["fullscreen"] = config.video.fullscreen;
  data["video"]["scale"] = config.video.scale;
  data["video"]["shader_vs"] = config.video.shader.path_vs;
  data["video"]["shader_fs"] = config.video.shader.path_fs;

  // Audio
  std::string resampler;
  switch (config.audio.interpolation) {
    case Config::Audio::Interpolation::Cosine: resampler = "cosine"; break;
    case Config::Audio::Interpolation::Cubic:  resampler = "cubic";  break;
    case Config::Audio::Interpolation::Sinc_64:  resampler = "sinc64"; break;
    case Config::Audio::Interpolation::Sinc_128: resampler = "sinc128"; break;
    case Config::Audio::Interpolation::Sinc_256: resampler = "sinc256"; break;
  }
  data["audio"]["resampler"] = resampler;
  data["audio"]["interpolate_fifo"] = config.audio.interpolate_fifo;
  data["audio"]["m4a_xq_enable"] = config.audio.m4a_xq_enable;

  std::ofstream file{ path, std::ios::out };
  file << data;
  file.close();
}

} // namespace nba
