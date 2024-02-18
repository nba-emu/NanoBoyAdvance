/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba {

#pragma pack(push, 1) 

// For details see: http://problemkaputt.de/gbatek.htm#gbacartridgeheader
struct Header {
  // First instruction
  u32 entrypoint;

  // Compressed Bitmap (Nintendo Logo)
  u8 nintendo_logo[156];

  // Game Title, Code and Maker Code
  struct {
    char title[12];
    char code[4];
    char maker[2];
  } game;

  u8 fixed_96h;
  u8 unit_code;
  u8 device_type;
  u8 reserved[7];
  u8 version;
  u8 checksum;
  u8 reserved2[2];

  // Multiboot Header
  struct {
    u32 ram_entrypoint;
    u8  boot_mode;
    u8  slave_id;
    u8  unused[26];
    u32 joy_entrypoint;
  } mb;
};

#pragma pack(pop)

} // namespace nba
