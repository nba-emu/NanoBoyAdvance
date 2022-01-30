/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>

#include "cheat_device.hpp"

namespace nba::core {

CheatDevice::CheatDevice(Bus& bus, Scheduler& scheduler)
    : bus(bus)
    , scheduler(scheduler) {
}

void CheatDevice::Reset() {
  scheduler.Add(kCyclesPerUpdate, this, &CheatDevice::Update);
}

void CheatDevice::Update(int late) {
  u64 codes[] = {
    // // Master ball
    // 0x958D8046A7151D70,
    // 0x8BB602F78CEB681A,

    // // Walk through walls
    // 0x7881A409E2026E0C,
    // 0x8E883EFF92E9660D,

    // Shiny pokemon
    0xF3A9A86D4E2629B4,
    0x18452A7DDDE55BCC

    // Encounter Mew
    // Does not work yet.
    // 0xB749822BCE9BFAC1,
    // 0xA86CDBA519BA49B3,
    // 0x401245E07D57E544
  };

  ExecuteARV3(codes, sizeof(codes)/sizeof(u64));

  scheduler.Add(kCyclesPerUpdate - late, this, &CheatDevice::Update);
}

void CheatDevice::ExecuteARV3(u64* codes, size_t size) {
  size_t i = 0;

  while (i < size) {
    // TODO: do not decrypt over and over again.
    u64 code = DecryptARV3(codes[i]);
    u32 l = code >> 32;
    u32 r = u32(code);

    i++;

    // See: http://problemkaputt.de/gbatek.htm#gbacheatcodesproactionreplayv3
    if ((l >> 24) == 0x02) {
      u32 address = (l & 0x000FFFFF) | ((l & 0x00F00000) << 4);
      u32 count = 1 + (r >> 16);
      u16 data = u16(r);

      u16* dst = (u16*)bus.GetHostAddress(address, count * sizeof(u16));

      for (u32 j = 0; j < count; j++) *dst++ = data;
    } else if (l == 0) {
      // TODO: add a way to undo ROM patches
      // TODO: do the ROM patch commands differ at all???
      // 00000000 18aaaaaa 0000zzzz 00000000  [8000000+aaaaaa*2]=zzzz  (ROM Patch 1)
      // 00000000 1Aaaaaaa 0000zzzz 00000000  [8000000+aaaaaa*2]=zzzz  (ROM Patch 2)
      // 00000000 1Caaaaaa 0000zzzz 00000000  [8000000+aaaaaa*2]=zzzz  (ROM Patch 3)
      // 00000000 1Eaaaaaa 0000zzzz 00000000  [8000000+aaaaaa*2]=zzzz  (ROM Patch 4)
      if ((r & 0xFF000000) == 0x18000000) {
        u32 address = 0x08000000 + ((r & 0xFFFFFF) << 1);
        u16 data = DecryptARV3(codes[i]) >> 32;
        *(u16*)bus.GetHostAddress(address, sizeof(u16)) = data;
        Log<Error>("Patch 0x{:08X} to 0x{:04X}", address, data);
        i++;
      } else {
        Log<Error>("Unhandled ARV3 cheat code: 0x{:016X}", code);
      }
    } else {
      Log<Error>("Unhandled ARV3 cheat code: 0x{:016X}", code);
    }
  }
}

auto CheatDevice::DecryptARV3(u64 code) -> u64 {
  const uint32_t S0 = 0x7AA9648F;
  const uint32_t S1 = 0x7FAE6994;
  const uint32_t S2 = 0xC0EFAAD5;
  const uint32_t S3 = 0x42712C57;

  u32 l = code >> 32;
  u32 r = u32(code);

  u32 tmp = 0x9E3779B9 << 5;

  for (int i = 0; i < 32; i++) {
    r -= ((l << 4) + S2) ^ (l + tmp) ^ ((l >> 5) + S3);
    l -= ((r << 4) + S0) ^ (r + tmp) ^ ((r >> 5) + S1);
    tmp -= 0x9E3779B9;
  }

  return (u64(l) << 32) | r;
}


} // namespace nba::core
