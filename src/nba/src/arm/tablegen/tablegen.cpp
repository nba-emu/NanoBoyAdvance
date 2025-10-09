/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/common/meta.hpp>

#include "arm/arm7tdmi.hpp"

namespace nba::core::arm {

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

    static_for<std::size_t, 0, 1024>([&](auto i) {
      lut[i] = GenerateHandlerThumb<i << 6>();
    });
    return lut;
  }

  static constexpr auto GenerateTableARM() -> std::array<Handler32, 4096> {
    std::array<Handler32, 4096> lut{};

    static_for<std::size_t, 0, 4096>([&](auto i) {
      lut[i] = GenerateHandlerARM<
        ((i & 0xFF0) << 16) |
        ((i & 0xF) << 4)>();
    });
    return lut;
  }

  static constexpr auto GenerateConditionTable() -> std::array<bool, 256> {
    std::array<bool, 256> lut{};
    
    for(int flag_set = 0; flag_set < 16; flag_set++) {
      bool n = flag_set & 8;
      bool z = flag_set & 4;
      bool c = flag_set & 2;
      bool v = flag_set & 1;

      lut[(COND_EQ << 4) | flag_set] =  z;
      lut[(COND_NE << 4) | flag_set] = !z;
      lut[(COND_CS << 4) | flag_set] =  c;
      lut[(COND_CC << 4) | flag_set] = !c;
      lut[(COND_MI << 4) | flag_set] =  n;
      lut[(COND_PL << 4) | flag_set] = !n;
      lut[(COND_VS << 4) | flag_set] =  v;
      lut[(COND_VC << 4) | flag_set] = !v;
      lut[(COND_HI << 4) | flag_set] =  c && !z;
      lut[(COND_LS << 4) | flag_set] = !c ||  z;
      lut[(COND_GE << 4) | flag_set] = n == v;
      lut[(COND_LT << 4) | flag_set] = n != v;
      lut[(COND_GT << 4) | flag_set] = !(z || (n != v));
      lut[(COND_LE << 4) | flag_set] =  (z || (n != v));
      lut[(COND_AL << 4) | flag_set] = true;
      lut[(COND_NV << 4) | flag_set] = false;
    }

    return lut;
  }
};

std::array<Handler16, 1024> ARM7TDMI::s_opcode_lut_16 = TableGen::GenerateTableThumb();
std::array<Handler32, 4096> ARM7TDMI::s_opcode_lut_32 = TableGen::GenerateTableARM();
std::array<bool, 256> ARM7TDMI::s_condition_lut = TableGen::GenerateConditionTable();

} // namespace nba::core::arm
