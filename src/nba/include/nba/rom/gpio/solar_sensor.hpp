/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/rom/gpio/device.hpp>

namespace nba {

struct SolarSensor final : GPIODevice {
  SolarSensor();

  void Reset() override;
  auto Read() -> int override;
  void Write(int value) override;
  void SetLightLevel(u8 level);

private:
  enum Pin {
    CLK = 0,
    RST = 1,
    nCS = 2,
    FLG = 3
  };

  int clk;
  u8 counter;
  u8 current_level;
};

} // namespace nba
