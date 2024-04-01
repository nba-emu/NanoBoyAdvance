/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>
#include <nba/rom/gpio/gpio.hpp>
#include <nba/rom/gpio/rtc.hpp>
#include <nba/rom/gpio/solar_sensor.hpp>

#include <nba/log.hpp>

namespace nba {

void GPIO::LoadState(SaveState const& state) {
  allow_reads = state.gpio.allow_reads;
  rd_mask =   state.gpio.rd_mask;
  wr_mask = (~state.gpio.rd_mask) & 15;
  port_data = state.gpio.port_data;

  for(auto& device : devices) {
    device->LoadState(state);
    device->SetPortDirections(wr_mask);
  }
}

void GPIO::CopyState(SaveState& state) {
  state.gpio.allow_reads = allow_reads;
  state.gpio.rd_mask = rd_mask;
  state.gpio.port_data = port_data;

  for(auto& device : devices) {
    device->CopyState(state);
  }
}

void RTC::LoadState(SaveState const& state) {
  current_bit = state.gpio.rtc.current_bit;
  current_byte = state.gpio.rtc.current_byte;
  reg = (Register)state.gpio.rtc.reg;
  data = state.gpio.rtc.data;
  this->state = (State)state.gpio.rtc.state;
  control.unknown1 = state.gpio.rtc.control.unknown1;
  control.per_minute_irq = state.gpio.rtc.control.per_minute_irq;
  control.unknown2 = state.gpio.rtc.control.unknown2;
  control.mode_24h = state.gpio.rtc.control.mode_24h;
  control.poweroff = state.gpio.rtc.control.poweroff;

  std::memcpy(buffer, state.gpio.rtc.buffer, sizeof(buffer));
}

void RTC::CopyState(SaveState& state) {
  state.gpio.rtc.current_bit = current_bit;
  state.gpio.rtc.current_byte = current_byte;
  state.gpio.rtc.reg = (u8)reg;
  state.gpio.rtc.data = data;
  state.gpio.rtc.state = (u8)this->state;
  state.gpio.rtc.control.unknown1 = control.unknown1;
  state.gpio.rtc.control.per_minute_irq = control.per_minute_irq;
  state.gpio.rtc.control.unknown2 = control.unknown2;
  state.gpio.rtc.control.mode_24h = control.mode_24h;
  state.gpio.rtc.control.poweroff = control.poweroff;

  std::memcpy(state.gpio.rtc.buffer, buffer, sizeof(buffer));
}

void SolarSensor::LoadState(SaveState const& state) {
  old_clk = state.gpio.solar_sensor.old_clk;
  counter = state.gpio.solar_sensor.counter;
}

void SolarSensor::CopyState(SaveState& state) {
  state.gpio.solar_sensor.old_clk = old_clk;
  state.gpio.solar_sensor.counter = counter;
}

} // namespace nba
