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
#include <emulator/core/interrupt.hpp>
#include <emulator/core/scheduler.hpp>

namespace nba::core {

class DMA {
public:
  using Access = arm::MemoryBase::Access;
  
  DMA(arm::MemoryBase* memory,
      InterruptController* irq_controller,
      Scheduler* scheduler) 
    : memory(memory)
    , irq_controller(irq_controller)
    , scheduler(scheduler)
  { Reset(); }
  
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
  bool IsRunning() { return runnable.any(); }
  auto GetOpenBusValue() -> std::uint32_t { return latch; }
  
private:
  constexpr bool CheckDestinationAddress(int chan_id, int page) {
    /* Only DMA3 may write to cartridge area. */
    return chan_id == 3 || page < 0x08;
  }
  
  constexpr bool CheckSourceAddress(int chan_id, int page) {
    /* DMA0 can not read ROM, but it is able to read the SRAM region. 
     * DMA explcitly disallows reading from BIOS memory,
     * therefore invoking DMA open bus instead of BIOS open bus.
     */
    return (chan_id != 0 || page < 0x08 || page >= 0x0E) && page >= 0x02;
  }
  
  constexpr int GetUnaliasedMemoryArea(int page) {
    if (page >= 0x09 && page <= 0x0D) {
      return 0x08;
    }
    
    if (page == 0x0F) {
      return 0x0E;
    }
    
    return page;
  }

  void TryStart(int chan_id);
  void OnChannelWritten(int chan_id, bool enabled_old);
  
  arm::MemoryBase* memory;
  InterruptController* irq_controller;
  Scheduler* scheduler;

  /* The id of the DMA that is currently running.
   * If no DMA is running, then the value will be minus one.
   */
  int current;
  
  /* Indicates whether the currently running DMA got
   * interleaved by a higher-priority DMA.
   */
  bool interleaved;
  
  /* Sets of HBLANK/VBLANK/VIDEO channels that will be
   * executable on the respective trigger/DMA request.
   */
  std::bitset<4> hblank_set;
  std::bitset<4> vblank_set;
  std::bitset<4> video_set;
  
  /* Set of DMA channels that may be executed. */
  std::bitset<4> runnable;
  
  /* The last value read by any DMA channel. Required for DMA open bus. */
  std::uint32_t latch;
  
  struct Channel {
    enum Control  {
      INCREMENT = 0,
      DECREMENT = 1,
      FIXED  = 2,
      RELOAD = 3
    };

    enum Timing {
      IMMEDIATE = 0,
      VBLANK  = 1,
      HBLANK  = 2,
      SPECIAL = 3
    };

    enum Size {
      HWORD = 0,
      WORD  = 1
    };

    int id;

    bool enable;
    bool repeat;
    bool interrupt;
    bool gamepak;

    std::uint16_t length;
    std::uint32_t dst_addr;
    std::uint32_t src_addr;
    Control dst_cntl;
    Control src_cntl;
    Timing time;
    Size size;

    bool allow_read;
    Access second_access;
    
    bool is_fifo_dma;
    
    struct {
      std::uint32_t length;
      std::uint32_t dst_addr;
      std::uint32_t src_addr;
    } latch;
  } channels[4];
};

} // namespace nba::core
