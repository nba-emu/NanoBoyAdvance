/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace GameBoyAdvance {

class CPU;
  
class Timer {

public:
  Timer(CPU* cpu) : cpu(cpu) { Reset(); }
  
  void Reset();
  auto Read (int chan_id, int offset) -> std::uint8_t;
  void Write(int chan_id, int offset, std::uint8_t value);
  void Run(int cycles);
  auto EstimateCyclesUntilIRQ() -> int;
  
private:
  void Increment(int chan_id, int increment);
  
  CPU* cpu;
  bool may_cause_irq;
  
  int run_list[4];
  int run_count;

  struct Channel {
    int id;

    struct Control {
      int frequency;
      bool cascade;
      bool interrupt;
      bool enable;
    } control;

    int cycles;
    std::uint16_t reload;
    std::uint32_t counter;

    /* Based on timer frequency. */
    int shift;
    int mask;
    
    bool cascades_into_timer_causing_irq;
  } channels[4];
};

}