/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/gpio/solar_sensor.hpp>
#include <nba/log.hpp>

namespace nba {

SolarSensor::SolarSensor() {
  Reset();
}

void SolarSensor::Reset() {
  clk = 0;
  counter = 0;
  SetLightLevel(0x50);
}

auto SolarSensor::Read() -> int {
  return (counter > current_level) ? (1 << Pin::FLG) : 0;
}

void SolarSensor::Write(int value) {
  // TODO: rename CLK member variable to old_clk
  auto old_clk = clk;

  if (GetPortDirection(Pin::nCS) == PortDirection::Out && (value & 4)) {
    // talking to the RTC; ignored
    Log<Error>("SolarSensor: talking to RTC...");
    return;
  }

  if (GetPortDirection(Pin::CLK) == PortDirection::Out) {
    clk = value & (1 << Pin::CLK);
  } else {
    Log<Error>("SolarSensor: CLK port should be set to 'output' but configured as 'input'.");
  }
  
  if (GetPortDirection(Pin::RST) == PortDirection::Out) {
    if (value & (1 << Pin::RST)) {
      counter = 0;
    }
  } else {
    Log<Error>("SolarSensor: RST port should be set to 'output' but configured as 'input'.");
  }

  if (old_clk && !clk) {
    counter++;
  }
}

void SolarSensor::SetLightLevel(u8 level) {
  current_level = level;
}

} // namespace nba
