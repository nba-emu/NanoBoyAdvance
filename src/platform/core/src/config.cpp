/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <platform/config.hpp>

namespace nba {

void PlatformConfig::Load(std::string const& path) {
  if (!std::filesystem::exists(path)) {
    Save(path);
    return;
  }

  toml::value data;

  try {
    data = toml::parse(path);
  } catch (std::exception& ex) {
    Log<Error>("Config: error while parsing TOML configuration: {0}", ex.what());
    return;
  }

  if (data.contains("general")) {
    auto general_result = toml::expect<toml::value>(data.at("general"));

    if (general_result.is_ok()) {
      auto general = general_result.unwrap();
      this->bios_path = toml::find_or<std::string>(general, "bios_path", "bios.bin");
      this->skip_bios = toml::find_or<toml::boolean>(general, "bios_skip", false);
      this->sync_to_audio = toml::find_or<toml::boolean>(general, "sync_to_audio", true);
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
        Log<Warn>("Config: backup type '{0}' is not valid, defaulting to auto-detect.", save_type);
        this->backup_type = Config::BackupType::Detect;
      } else {
        this->backup_type = match->second;
      }

      this->force_rtc = toml::find_or<toml::boolean>(cartridge, "force_rtc", false);
    }
  }

  if (data.contains("video")) {
    auto video_result = toml::expect<toml::value>(data.at("video"));

    if (video_result.is_ok()) {
      auto video = video_result.unwrap();
  
      this->video.fullscreen = toml::find_or<toml::boolean>(video, "fullscreen", false);
      this->video.scale = toml::find_or<int>(video, "scale", 2);
      this->video.shader.path_vs = toml::find_or<std::string>(video, "shader_vs", "");
      this->video.shader.path_fs = toml::find_or<std::string>(video, "shader_fs", "");

      const std::map<std::string, Video::Filter> filters{
        { "nearest", Video::Filter::Nearest },
        { "linear",  Video::Filter::Linear  },
        { "xbrz",    Video::Filter::xBRZ    }
      };

      const std::map<std::string, Video::Color> color_corrections{
        { "none", Video::Color::No  },
        { "agb",  Video::Color::AGB },
        { "ags",  Video::Color::AGS }
      };

      auto filter = toml::find_or<std::string>(video, "filter", "nearest");
      auto filter_match = filters.find(filter);
      if (filter_match != filters.end()) {
        this->video.filter = filter_match->second;
      }

      auto color_correction = toml::find_or<std::string>(video, "color_correction", "ags");
      auto color_correction_match = color_corrections.find(color_correction);
      if (color_correction_match != color_corrections.end()) {
        this->video.color = color_correction_match->second;  
      }

      this->video.lcd_ghosting = toml::find_or<bool>(video, "lcd_ghosting", true);
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
        Log<Warn>("Config: unknown resampling algorithm: {} (defaulting to cosine).", resampler);
        this->audio.interpolation = Config::Audio::Interpolation::Cosine;
      } else {
        this->audio.interpolation = match->second;
      }

      this->audio.interpolate_fifo = toml::find_or<toml::boolean>(audio, "interpolate_fifo", true);
      this->audio.mp2k_hle_enable = toml::find_or<toml::boolean>(audio, "mp2k_hle_enable", false);
      this->audio.mp2k_hle_cubic = toml::find_or<toml::boolean>(audio, "mp2k_hle_cubic", false);
    }
  }

  LoadCustomData(data);
}

void PlatformConfig::Save(std::string const& path) {
  toml::basic_value<toml::preserve_comments> data;

  if (std::filesystem::exists(path)) {
    try {
      data = toml::parse<toml::preserve_comments>(path);
    } catch (std::exception& ex) {
      Log<Error>("Config: error while parsing TOML configuration: {0}", ex.what());
      return;
    }
  }

  // General
  data["general"]["bios_path"] = this->bios_path;
  data["general"]["bios_skip"] = this->skip_bios;
  data["general"]["sync_to_audio"] = this->sync_to_audio;

  // Cartridge
  std::string save_type;
  switch (this->backup_type) {
    case Config::BackupType::Detect: save_type = "detect"; break;
    case Config::BackupType::None:   save_type = "none"; break;
    case Config::BackupType::SRAM:   save_type = "sram"; break;
    case Config::BackupType::FLASH_64:  save_type = "flash64"; break;
    case Config::BackupType::FLASH_128: save_type = "flash128"; break;
    case Config::BackupType::EEPROM_4:  save_type = "eeprom512"; break;
    case Config::BackupType::EEPROM_64: save_type = "eeprom8192"; break;
  }
  data["cartridge"]["save_type"] = save_type;
  data["cartridge"]["force_rtc"] = this->force_rtc;

  // Video
  std::string filter;
  std::string color_correction;

  switch (this->video.filter) {
    case Video::Filter::Nearest: filter = "nearest"; break;
    case Video::Filter::Linear:  filter = "linear"; break;
    case Video::Filter::xBRZ:    filter = "xbrz"; break; 
  }

  switch (this->video.color) {
    case Video::Color::No:  color_correction = "none"; break;
    case Video::Color::AGB: color_correction = "agb"; break;
    case Video::Color::AGS: color_correction = "ags"; break;
  }

  data["video"]["fullscreen"] = this->video.fullscreen;
  data["video"]["scale"] = this->video.scale;
  data["video"]["shader_vs"] = this->video.shader.path_vs;
  data["video"]["shader_fs"] = this->video.shader.path_fs;
  data["video"]["filter"] = filter;
  data["video"]["color_correction"] = color_correction;
  data["video"]["lcd_ghosting"] = this->video.lcd_ghosting;

  // Audio
  std::string resampler;
  switch (this->audio.interpolation) {
    case Config::Audio::Interpolation::Cosine: resampler = "cosine"; break;
    case Config::Audio::Interpolation::Cubic:  resampler = "cubic";  break;
    case Config::Audio::Interpolation::Sinc_64:  resampler = "sinc64"; break;
    case Config::Audio::Interpolation::Sinc_128: resampler = "sinc128"; break;
    case Config::Audio::Interpolation::Sinc_256: resampler = "sinc256"; break;
  }
  data["audio"]["resampler"] = resampler;
  data["audio"]["interpolate_fifo"] = this->audio.interpolate_fifo;
  data["audio"]["mp2k_hle_enable"] = this->audio.mp2k_hle_enable;
  data["audio"]["mp2k_hle_cubic"] = this->audio.mp2k_hle_cubic;

  SaveCustomData(data);

  std::ofstream file{ path, std::ios::out };
  file << data;
  file.close();
}

} // namespace nba
