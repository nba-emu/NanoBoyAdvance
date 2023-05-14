/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <nba/config.hpp>
#include <nba/save_state.hpp>
#include <memory>

#include "hw/irq/irq.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct KeyPad {
  KeyPad(Scheduler& scheduler, IRQ& irq, std::shared_ptr<Config> config);

  void Reset();

  struct KeyInput {
    u16 value = 0x3FF;

    auto ReadByte(uint offset) -> u8;
  } input;

  struct KeyControl {
    u16 mask;
    bool interrupt;

    enum class Mode {
      LogicalOR  = 0,
      LogicalAND = 1
    } mode = Mode::LogicalOR;
  
    KeyPad* keypad;

    auto ReadByte(uint offset) -> u8;
    void WriteByte(uint offset, u8 value);
    void WriteHalf(u16 value);
  } control;

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

private:
  static constexpr int k_poll_interval = 4096;

  using Key = InputDevice::Key;

  void UpdateInput();
  void UpdateIRQ();

  void Poll();

  Scheduler& scheduler;
  IRQ& irq;
  std::shared_ptr<Config> config;

  struct InputQueue {
    static constexpr std::size_t k_buffer_size = 16;

    std::atomic<u16> data[k_buffer_size];
    std::atomic<std::size_t> size = 0;
    std::size_t rd_ptr = 0;
    std::size_t wr_ptr = 0;

    bool DataAvailable();
    void Enqueue(u16 input);
    auto Dequeue() -> u16;
  } input_queue{};
};

} // namespace nba::core
