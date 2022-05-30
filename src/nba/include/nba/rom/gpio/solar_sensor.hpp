/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/rom/gpio/gpio.hpp>

namespace nba {

struct SolarSensor : GPIO {
  SolarSensor() {
    Reset();
  }

  void Reset();
  void SetCurrentLightLevel(u8 level);

protected:
  auto ReadPort() -> u8 final;
  void WritePort(u8 value) final;

private:
  int clk;
  u8 counter;
  u8 current_level;
};

} // namespace nba
