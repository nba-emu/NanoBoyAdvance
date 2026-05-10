// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/rom/gpio/device.hh>

namespace nba {

struct SolarSensor final : GPIODevice {
  SolarSensor();

  void Reset() override;
  auto Read() -> int override;
  void Write(int value) override;
  void SetLightLevel(u8 level);

  void LoadState(SaveState const& state) override;
  void CopyState(SaveState& state) override;

private:
  enum Pin {
    CLK = 0,
    RST = 1,
    nCS = 2,
    FLG = 3
  };

  bool old_clk;
  u8 counter;
  u8 current_level;
};

} // namespace nba
