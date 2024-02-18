/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>
#include <nba/save_state.hpp>

namespace nba {

struct GPIODevice {
  enum class PortDirection {
    In  = 0, // GPIO -> GBA
    Out = 1  // GPIO <- GBA
  };

  virtual ~GPIODevice() = default;

  virtual void Reset() = 0;
  virtual auto Read() -> int = 0;
  virtual void Write(int value) = 0;

  virtual void LoadState(SaveState const& state) {}
  virtual void CopyState(SaveState& state) {}
  
  void SetPortDirections(int port_directions) {
    this->port_directions = port_directions;
  }

  auto GetPortDirection(int pin) const -> PortDirection {
    return static_cast<PortDirection>((port_directions >> pin) & 1);
  }

private:
  int port_directions = 0;
};

} // namespace nba
