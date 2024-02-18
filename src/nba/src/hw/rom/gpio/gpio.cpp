/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/gpio/gpio.hpp>

namespace nba {

void GPIO::Reset() {
  allow_reads = false;
  port_data = 0;
  rd_mask = 0b1111;
  wr_mask = 0b0000;

  for(auto& device : devices) {
    device->Reset();
    device->SetPortDirections(0);
  }
}

void GPIO::Attach(std::shared_ptr<GPIODevice> device) {
  devices.push_back(device);
  device_map[std::type_index{typeid(*device)}] = device.get();
}

auto GPIO::Read(u32 address) -> u8 {
  if(!allow_reads) {
    return 0;
  }

  switch(static_cast<Register>(address)) {
    case Register::Data: {
      u8 value = 0;

      for(auto& device : devices) {
        value |= device->Read();
      }

      port_data &= wr_mask;
      port_data |= rd_mask & value;
      return value;
    }
    case Register::Direction: {
      return rd_mask;
    }
    case Register::Control: {
      return allow_reads ? 1 : 0;
    }
  }

  return 0;
}

void GPIO::Write(u32 address, u8 value) {
  switch(static_cast<Register>(address)) {
    case Register::Data: {
      port_data &= rd_mask;
      port_data |= wr_mask & value;

      for(auto& device : devices) {
        device->Write(port_data);
      }
      break;
    }
    case Register::Direction: {
      value &= 15;

      rd_mask = ~value & 15;
      wr_mask =  value;

      for(auto& device : devices) {
        device->SetPortDirections(value);
      }
      break;
    }
    case Register::Control: {
      allow_reads = value & 1;
      break;
    }
  }
}

} // namespace nba
