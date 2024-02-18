/*
 * Copyright (C) 2024 fleroviux
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
  old_clk = false;
  counter = 0;
  SetLightLevel(0x60);
}

auto SolarSensor::Read() -> int {
  return (counter > current_level) ? (1 << Pin::FLG) : 0;
}

void SolarSensor::Write(int value) {
  bool clk = (value & (1 << Pin::CLK)) && GetPortDirection(Pin::CLK) == PortDirection::Out;
  bool rst = (value & (1 << Pin::RST)) && GetPortDirection(Pin::RST) == PortDirection::Out;

  if(rst) {
    counter = 0;
  } else if(old_clk && !clk) {
    counter++;
  }

  old_clk = clk;
}

void SolarSensor::SetLightLevel(u8 level) {
  current_level = 255 - level;
}

} // namespace nba
