/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cassert>
#include <emulator/core/scheduler.hpp>
#include <emulator/core/interrupt.hpp>

namespace nba {

class GPIO {
public:
  // TODO: port directions are described from the perspective
  // of the GBA. Reflect this in the naming.
  enum class PortDirection {
    In  = 0, // GPIO -> GBA
    Out = 1  // GPIO <- GBA
  };

  GPIO(nba::core::Scheduler* scheduler, nba::core::InterruptController* irq_controller)
    : scheduler(scheduler)
    , irq_controller(irq_controller) 
  { Reset(); }

  virtual ~GPIO() {}

  void Reset();

  auto GetPortDirection(int port) const -> PortDirection {
    assert(port < 4);
    return direction[port];
  }

  bool IsReadable() const {
    return allow_reads;
  }

  auto Read (std::uint32_t address) -> std::uint8_t;
  void Write(std::uint32_t address, std::uint8_t value);

protected:
  virtual auto ReadPort() -> std::uint8_t = 0;
  virtual void WritePort(std::uint8_t value) = 0;

  nba::core::Scheduler* scheduler;
  nba::core::InterruptController* irq_controller;

private:
  enum class Register {
    Data = 0xC4,
    Direction = 0xC6,
    Control = 0xC8
  };

  void UpdateReadWriteMasks();

  bool allow_reads;
  PortDirection direction[4];
  std::uint8_t rd_mask;
  std::uint8_t wr_mask;
  std::uint8_t port_data;
};

} // namespace nba
