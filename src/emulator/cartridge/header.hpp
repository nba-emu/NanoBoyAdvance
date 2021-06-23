/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba {

#pragma pack(push, 1) 

// For details see: http://problemkaputt.de/gbatek.htm#gbacartridgeheader
struct Header {
  // First instruction
  std::uint32_t entrypoint;

  // Compressed Bitmap (Nintendo Logo)
  std::uint8_t nintendo_logo[156];

  // Game Title, Code and Maker Code
  struct {
    char title[12];
    char code[4];
    char maker[2];
  } game;

  std::uint8_t fixed_96h;
  std::uint8_t unit_code;
  std::uint8_t device_type;
  std::uint8_t reserved[7];
  std::uint8_t version;
  std::uint8_t checksum;
  std::uint8_t reserved2[2];

  // Multiboot Header
  struct {
    std::uint32_t ram_entrypoint;
    std::uint8_t  boot_mode;
    std::uint8_t  slave_id;
    std::uint8_t  unused[26];
    std::uint32_t joy_entrypoint;
  } mb;
};

#pragma pack(pop)

} // namespace nba
