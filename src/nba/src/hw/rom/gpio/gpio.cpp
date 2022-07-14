/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/gpio/gpio.hpp>

namespace nba {

void GPIO::Reset() {
  allow_reads = false;
  port_data = 0;
  for (int i = 0; i < 4; i++) {
    direction[i] = PortDirection::Out;
  }
  UpdateReadWriteMasks();
}

void GPIO::Attach(std::shared_ptr<GPIODevice> device) {
  devices.push_back(device);
}

void GPIO::UpdateReadWriteMasks() {
  rd_mask = 0;
  for (int i = 0; i < 4; i++) {
    if (GetPortDirection(i) == PortDirection::In) rd_mask |= 1 << i;
  }
  wr_mask = ~rd_mask & 15;
}

auto GPIO::Read(u32 address) -> u8 {
  if (!allow_reads) {
    return 0;
  }

  switch (static_cast<Register>(address)) {
    case Register::Data: {
      //auto value = ReadPort() & rd_mask;
      u8 value = ReadPort();

      for (auto& device : devices) {
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
  switch (static_cast<Register>(address)) {
    case Register::Data: {
      port_data &= rd_mask;
      port_data |= wr_mask & value;
      WritePort(port_data);

      for (auto& device : devices) {
        device->Write(port_data);
      }
      break;
    }
    case Register::Direction: {
      // TODO: simplify this, rd_mask and wr_mask correspond directly to value.
      for (int i = 0; i < 4; i++) {
        direction[i] = (value & (1 << i)) ? PortDirection::Out : PortDirection::In;
      }
      UpdateReadWriteMasks();

      value &= 15;

      for (auto& device : devices) {
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
