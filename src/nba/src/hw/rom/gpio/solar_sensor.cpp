/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>

#include "solar_sensor.hpp"

namespace nba {

void SolarSensor::Reset() {
  // TODO: get rid of this weird construct both in the solar sensor and the RTC.
  GPIO::Reset();

  clk = 0;
  counter = 0;
  SetCurrentLightLevel(0x50);
}

void SolarSensor::SetCurrentLightLevel(u8 level) {
  current_level = level;
}

auto SolarSensor::ReadPort() -> u8 {
  return (counter > current_level) ? 8 : 0;
}

void SolarSensor::WritePort(u8 value) {
  auto old_clk = clk;

  // if (GetPortDirection(2) == PortDirection::Out && (value & 4)) {
  //   // talking to the RTC; ignored
  //   return;
  // }

  if (GetPortDirection(0) == PortDirection::Out) {
    clk = value & 1;
  } else {
    Log<Error>("SolarSensor: CLK port should be set to 'output' but configured as 'input'.");
  }
  
  if (GetPortDirection(1) == PortDirection::Out) {
    if (value & 2) {
      counter = 0;
    }
  } else {
    Log<Error>("SolarSensor: RST port should be set to 'output' but configured as 'input'.");
  }

  if (old_clk && !clk) {
    counter++;
  }
}

} // namespace nba
