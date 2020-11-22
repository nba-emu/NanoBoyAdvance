/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <bitset>
#include <cstdint>
#include <emulator/core/arm/memory.hpp>
#include <emulator/core/hw/interrupt.hpp>
#include <emulator/core/scheduler.hpp>

namespace nba::core {

class DMA {
public:
  using Access = arm::MemoryBase::Access;

  DMA(arm::MemoryBase& memory, IRQ& irq, Scheduler& scheduler)
      : memory(memory)
      , irq(irq)
      , scheduler(scheduler) {
    Reset();
  }

  enum class Occasion {
    HBlank,
    VBlank,
    Video,
    FIFO0,
    FIFO1
  };

  void Reset();
  void Request(Occasion occasion);
  void StopVideoXferDMA();
  void Run();
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);
  bool IsRunning() { return runnable_set.any(); }
  auto GetOpenBusValue() -> std::uint32_t { return latch; }

private:
  enum Registers {
    REG_DMAXSAD = 0,
    REG_DMAXDAD = 4,
    REG_DMAXCNT_L = 8,
    REG_DMAXCNT_H = 10
  };

  struct Channel {
    int id;
    bool enable = false;
    bool repeat = false;
    bool interrupt = false;
    bool gamepak = false;

    std::uint16_t length = 0;
    std::uint32_t dst_addr = 0;
    std::uint32_t src_addr = 0;

    enum Control  {
      Increment = 0,
      Decrement = 1,
      Fixed  = 2,
      Reload = 3
    } dst_cntl = Increment, src_cntl = Increment;

    enum Timing {
      Immediate = 0,
      VBlank  = 1,
      HBlank  = 2,
      Special = 3
    } time = Immediate;

    enum Size {
      Half = 0,
      Word  = 1
    } size = Half;

    struct Latch {
      std::uint32_t length;
      std::uint32_t dst_addr;
      std::uint32_t src_addr;
    } latch;

    bool is_fifo_dma = false;
    Scheduler::Event* startup_event = nullptr;
  } channels[4];

  constexpr int GetUnaliasedMemoryArea(int page) {
    if (page >= 0x09 && page <= 0x0D) {
      return 0x08;
    }

    if (page == 0x0F) {
      return 0x0E;
    }

    return page;
  }

  void ScheduleDMAs(unsigned int bitset);
  void SelectNextDMA();
  void OnChannelWritten(Channel& channel, bool enable_old);
  void RunChannel(bool first);

  arm::MemoryBase& memory;
  IRQ& irq;
  Scheduler& scheduler;

  int active_dma_id;
  bool early_exit_trigger;

  /// Set of currently enabled H-blank DMAs.
  std::bitset<4> hblank_set;

  /// Set of currently enabled V-blank DMAs.
  std::bitset<4> vblank_set;

  /// Set of currently enabled video transfer DMAs.
  std::bitset<4> video_set;

  /// Set of DMAs which are currently scheduled for execution.
  std::bitset<4> runnable_set;

  /// Most recent value transferred by any DMA channel.
  /// When attempting read from illegal addresses, this value will be read instead.
  std::uint32_t latch;
};

} // namespace nba::core
