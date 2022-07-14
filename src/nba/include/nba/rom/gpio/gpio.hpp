/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cassert>
#include <nba/rom/gpio/device.hpp>
#include <nba/integer.hpp>
#include <memory>
#include <vector>

// TODO: rethink usage of std::shared_ptr for GPIODevices

namespace nba {

struct GPIO {
  enum class PortDirection {
    In  = 0, // GPIO -> GBA
    Out = 1  // GPIO <- GBA
  };

  GPIO() {
    Reset();
  }

  virtual ~GPIO() = default;

  void Reset();
  void Attach(std::shared_ptr<GPIODevice> device);

  auto GetPortDirection(int port) const -> PortDirection {
    assert(port < 4);
    return direction[port];
  }

  bool IsReadable() const {
    return allow_reads;
  }

  auto Read (u32 address) -> u8;
  void Write(u32 address, u8 value);

protected:
  virtual auto ReadPort() -> u8 = 0;
  virtual void WritePort(u8 value) = 0;

private:
  enum class Register {
    Data = 0xC4,
    Direction = 0xC6,
    Control = 0xC8
  };

  void UpdateReadWriteMasks();

  bool allow_reads;
  PortDirection direction[4];
  u8 rd_mask;
  u8 wr_mask;
  u8 port_data;

  std::vector<std::shared_ptr<GPIODevice>> devices;
};

} // namespace nba
