// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_config.hh"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>

namespace nba {

void DreamcastConfig::ApplyDefaults() {
  bios_path = kDefaultBIOSPath;
  save_folder = kDefaultSaveFolder;
  rom_folder = kDefaultROMFolder;
  video.filter = PlatformConfig::Video::Filter::Nearest;
  video.color = PlatformConfig::Video::Color::No;
  show_fps = false;
  allow_large_roms = false;

  // The Balanced profile seeds the CPU/audio/video knobs (mp2k HLE,
  // interpolation, frame skip, audio buffer, LCD ghosting).
  ApplyPerformanceProfile(PerformanceProfile::Balanced);
}

void DreamcastConfig::ApplyPerformanceProfile(PerformanceProfile profile) {
  performance_profile = profile;

  switch(profile) {
    case PerformanceProfile::Accuracy:
      // Cycle-accurate audio mixing and high-order interpolation. No frame
      // skipping; a larger audio buffer absorbs the heavier CPU load.
      audio.mp2k_hle_enable = false;
      audio.interpolation = Config::Audio::Interpolation::Sinc_64;
      video.lcd_ghosting = true;
      frame_skip = 0;
      audio_buffer_size = 8192;
      break;

    case PerformanceProfile::Balanced:
      // Native audio mixing with cheap interpolation and no frame skipping.
      audio.mp2k_hle_enable = false;
      audio.interpolation = Config::Audio::Interpolation::Cosine;
      video.lcd_ghosting = false;
      frame_skip = 0;
      audio_buffer_size = 4096;
      break;

    case PerformanceProfile::Speed:
      // HLE audio skips the GBA sound CPU; one frame of skip and a deeper
      // buffer reclaim headroom on the most demanding titles.
      audio.mp2k_hle_enable = true;
      audio.interpolation = Config::Audio::Interpolation::Cosine;
      video.lcd_ghosting = false;
      frame_skip = 1;
      audio_buffer_size = 8192;
      break;
  }
}

auto DreamcastConfig::ProfileName(PerformanceProfile profile) -> const char* {
  switch(profile) {
    case PerformanceProfile::Accuracy: return "Accuracy";
    case PerformanceProfile::Speed:    return "Speed";
    case PerformanceProfile::Balanced:
    default:                           return "Balanced";
  }
}

auto DreamcastConfig::ProfileFromName(std::string const& name, PerformanceProfile fallback)
  -> PerformanceProfile {
  if(name == "Accuracy") return PerformanceProfile::Accuracy;
  if(name == "Balanced") return PerformanceProfile::Balanced;
  if(name == "Speed")    return PerformanceProfile::Speed;
  return fallback;
}

void DreamcastConfig::LoadDreamcast(std::string const& path) {
  ApplyDefaults();

  if(std::filesystem::exists(path)) {
    Load(path);
    return;
  }

  SaveDreamcast(path);
}

void DreamcastConfig::LoadDreamcastSafe(std::string const& path) {
  ApplyDefaults();

  // Probe for the config file with fopen rather than std::filesystem, which can
  // hang or misbehave on Flycast's /pc/ virtual filesystem.  If the file is
  // absent or unreadable, keep defaults and intentionally do NOT write a new
  // file here (a write could hang on the same media); settings are persisted
  // later when the user saves from the settings menu.
  auto* file = std::fopen(path.c_str(), "rb");
  if(!file) {
    std::printf("[NBA-DC] Config: none found at %s, using defaults\n", path.c_str());
    std::fflush(stdout);
    return;
  }

  std::string content;
  char buffer[1024];
  size_t read = 0;
  while((read = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
    content.append(buffer, read);
  }
  std::fclose(file);

  if(content.empty()) {
    std::printf("[NBA-DC] Config: empty file at %s, using defaults\n", path.c_str());
    std::fflush(stdout);
    return;
  }

  // Parse from the in-memory buffer so toml never touches the filesystem.  On a
  // malformed file, fall back cleanly to defaults instead of a partial state.
  try {
    auto data = toml::parse_str(content);
    LoadFromData(data);
    std::printf("[NBA-DC] Config: loaded %s\n", path.c_str());
    std::fflush(stdout);
  } catch(std::exception const& ex) {
    ApplyDefaults();
    std::printf("[NBA-DC] Config: parse error in %s (%s), using defaults\n",
                path.c_str(), ex.what());
    std::fflush(stdout);
  }
}

void DreamcastConfig::SaveDreamcast(std::string const& path) {
  // Route through the guarded writer so settings saves never hit the
  // std::filesystem / read-modify-write path that can hang on Flycast.
  SaveDreamcastSafe(path);
}

auto DreamcastConfig::SaveDreamcastSafe(std::string const& path) -> bool {
  // Build the full document fresh in memory.  Unlike the base Save path we do
  // not read and merge the existing file (a parse that can hang on /pc/); the
  // Dreamcast config writes every key it owns, so nothing is lost.
  std::string content;
  try {
    toml::value data;
    SaveToData(data);
    std::ostringstream oss;
    oss << data;
    content = oss.str();
  } catch(std::exception const& ex) {
    std::printf("[NBA-DC] Config: serialize error for %s (%s)\n", path.c_str(), ex.what());
    std::fflush(stdout);
    return false;
  }

  auto* file = std::fopen(path.c_str(), "wb");
  if(!file) {
    std::printf("[NBA-DC] Config: could not open %s for write\n", path.c_str());
    std::fflush(stdout);
    return false;
  }

  const size_t written = std::fwrite(content.data(), 1, content.size(), file);
  const bool flushed = std::fflush(file) == 0;
  const bool closed = std::fclose(file) == 0;
  const bool ok = written == content.size() && flushed && closed;

  std::printf("[NBA-DC] Config: save %s %s (%lu bytes)\n",
              path.c_str(), ok ? "ok" : "failed",
              static_cast<unsigned long>(content.size()));
  std::fflush(stdout);
  return ok;
}

void DreamcastConfig::LoadCustomData(toml::value const& data) {
  if(!data.contains("dreamcast")) {
    return;
  }

  auto dreamcast = data.at("dreamcast");
  performance_profile = ProfileFromName(
    toml::find_or<std::string>(dreamcast, "performance_profile", ProfileName(performance_profile)),
    performance_profile
  );
  frame_skip = toml::find_or<int>(dreamcast, "frame_skip", frame_skip);
  audio_buffer_size = toml::find_or<int>(dreamcast, "audio_buffer_size", audio_buffer_size);
  show_fps = toml::find_or<bool>(dreamcast, "show_fps", show_fps);
  allow_large_roms = toml::find_or<bool>(dreamcast, "allow_large_roms", allow_large_roms);
  rom_folder = toml::find_or<std::string>(dreamcast, "rom_folder", rom_folder);
  last_rom = toml::find_or<std::string>(dreamcast, "last_rom", last_rom);

  frame_skip = std::clamp(frame_skip, 0, 3);

  if(audio_buffer_size != 2048 && audio_buffer_size != 4096 && audio_buffer_size != 8192) {
    audio_buffer_size = 4096;
  }
}

void DreamcastConfig::SaveCustomData(toml::value& data) {
  data["dreamcast"]["performance_profile"] = ProfileName(performance_profile);
  data["dreamcast"]["frame_skip"] = frame_skip;
  data["dreamcast"]["audio_buffer_size"] = audio_buffer_size;
  data["dreamcast"]["show_fps"] = show_fps;
  data["dreamcast"]["allow_large_roms"] = allow_large_roms;
  data["dreamcast"]["rom_folder"] = rom_folder;
  data["dreamcast"]["last_rom"] = last_rom;
}

} // namespace nba
