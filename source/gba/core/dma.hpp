/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <bitset>
#include <cstdint>

namespace GameBoyAdvance {

class CPU;
  
class DMA {

public:
  DMA(CPU* cpu) : cpu(cpu) { Reset(); }
  
  enum class Occasion {
    HBlank,
    VBlank,
    Video,
    FIFO0,
    FIFO1
  };
  
  void Reset();
  void Request(Occasion occasion);
  void Run(int const& ticks_left);
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);
  bool IsRunning() { return runnable.any(); }
  
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
  
private:
  void TryStart(int chan_id);
  void OnChannelWritten(int chan_id, bool enabled_old);
  
  CPU* cpu;
  
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
  
  /* Set DMA channels that may be executed. */
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
    ARM::AccessType second_access;
    
    bool is_fifo_dma;
    int fifo_request_count;
    
    struct {
      std::uint32_t length;
      std::uint32_t dst_addr;
      std::uint32_t src_addr;
    } latch;
  } channels[4];
};

}