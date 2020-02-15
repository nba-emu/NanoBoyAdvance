/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/static_for.hpp>

#include "../arm7tdmi.hpp"

namespace ARM {

using Handler16 = ARM7TDMI::Handler16;
using Handler32 = ARM7TDMI::Handler32;

/** A helper class used to generate lookup tables for
  * the interpreter at compiletime.
  * The motivation is to separate the code used for generation from
  * the interpreter class and its header itself.
  */
struct TableGen {
  #ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Weverything"
  #endif

  #include "gen_arm.hpp"
  #include "gen_thumb.hpp"

  #ifdef __clang__
  #pragma clang diagnostic pop
  #endif

  static constexpr auto GenerateTableThumb() -> std::array<Handler16, 1024> {
    std::array<Handler16, 1024> lut{};

    Common::static_for<std::size_t, 0, 1024>([&](auto i) {
      lut[i] = GenerateHandlerThumb<i << 6>();
    });

    return lut;
  }

  static constexpr auto GenerateTableARM() -> std::array<Handler32, 4096> {
    std::array<Handler32, 4096> lut{};

    Common::static_for<std::size_t, 0, 4096>([&](auto i) {
      lut[i] = GenerateHandlerARM<
        ((i & 0xFF0) << 16) |
        ((i & 0xF) << 4)>();
    });

    return lut;
  }
};

std::array<Handler16, 1024> ARM7TDMI::s_opcode_lut_16 = TableGen::GenerateTableThumb();
std::array<Handler32, 4096> ARM7TDMI::s_opcode_lut_32 = TableGen::GenerateTableARM();

} // namespace ARM
