/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>
#include <nba/rom/gpio/rtc.hpp>
#include <nba/rom/gpio/solar_sensor.hpp>
#include <nba/rom/rom.hpp>
#include <nba/config.hpp>
#include <nba/integer.hpp>
#include <nba/save_state.hpp>
#include <nba/scheduler.hpp>
#include <vector>

namespace nba {

enum class Key : u8 {
  A = 0,
  B = 1,
  Select = 2,
  Start = 3,
  Right = 4,
  Left = 5,
  Up = 6,
  Down = 7,
  R = 8,
  L = 9,
  Count = 10
};

struct CoreBase {
  static constexpr int kCyclesPerFrame = 280896;

  virtual ~CoreBase() = default;

  virtual void Reset() = 0;

  virtual void Attach(std::vector<u8> const& bios) = 0;
  virtual void Attach(ROM&& rom) = 0;
  virtual auto CreateRTC() -> std::unique_ptr<RTC> = 0;
  virtual auto CreateSolarSensor() -> std::unique_ptr<SolarSensor> = 0;
  virtual void LoadState(SaveState const& state) = 0;
  virtual void CopyState(SaveState& state) = 0;
  virtual void SetKeyStatus(Key key, bool pressed) = 0;
  virtual void Run(int cycles) = 0;

  virtual auto GetROM() -> ROM& = 0;
  virtual auto GetPRAM() -> u8* = 0;
  virtual auto GetVRAM() -> u8* = 0;
  virtual auto GetOAM() -> u8* = 0;
  // @todo: come up with a solution for reading write-only registers.
  virtual auto PeekByteIO(u32 address) -> u8  = 0;
  virtual auto PeekHalfIO(u32 address) -> u16 = 0;
  virtual auto PeekWordIO(u32 address) -> u32 = 0;
  virtual auto GetBGHOFS(int id) -> u16 = 0;
  virtual auto GetBGVOFS(int id) -> u16 = 0;

  virtual core::Scheduler& GetScheduler() = 0;

  void RunForOneFrame() {
    Run(kCyclesPerFrame);
  }
};

auto CreateCore(
  std::shared_ptr<Config> config
) -> std::unique_ptr<CoreBase>;

} // namespace nba