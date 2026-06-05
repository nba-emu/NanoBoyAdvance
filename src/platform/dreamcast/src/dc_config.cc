// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_config.hh"

#include <algorithm>

namespace nba {

void DreamcastConfig::ApplyDefaults() {
  bios_path = kDefaultBIOSPath;
  save_folder = kDefaultSaveFolder;
  rom_folder = kDefaultROMFolder;
  audio.mp2k_hle_enable = false;
  audio.interpolation = Config::Audio::Interpolation::Cosine;
  video.filter = PlatformConfig::Video::Filter::Nearest;
  video.color = PlatformConfig::Video::Color::No;
  video.lcd_ghosting = false;
  frame_skip = 0;
  audio_buffer_size = 4096;
}

void DreamcastConfig::LoadDreamcast(std::string const& path) {
  ApplyDefaults();

  if(std::filesystem::exists(path)) {
    Load(path);
    return;
  }

  SaveDreamcast(path);
}

void DreamcastConfig::SaveDreamcast(std::string const& path) {
  Save(path);
}

void DreamcastConfig::LoadCustomData(toml::value const& data) {
  if(!data.contains("dreamcast")) {
    return;
  }

  auto dreamcast = data.at("dreamcast");
  frame_skip = toml::find_or<int>(dreamcast, "frame_skip", frame_skip);
  audio_buffer_size = toml::find_or<int>(dreamcast, "audio_buffer_size", audio_buffer_size);
  rom_folder = toml::find_or<std::string>(dreamcast, "rom_folder", rom_folder);
  last_rom = toml::find_or<std::string>(dreamcast, "last_rom", last_rom);

  frame_skip = std::clamp(frame_skip, 0, 3);

  if(audio_buffer_size != 2048 && audio_buffer_size != 4096 && audio_buffer_size != 8192) {
    audio_buffer_size = 4096;
  }
}

void DreamcastConfig::SaveCustomData(toml::value& data) {
  data["dreamcast"]["frame_skip"] = frame_skip;
  data["dreamcast"]["audio_buffer_size"] = audio_buffer_size;
  data["dreamcast"]["rom_folder"] = rom_folder;
  data["dreamcast"]["last_rom"] = last_rom;
}

} // namespace nba
