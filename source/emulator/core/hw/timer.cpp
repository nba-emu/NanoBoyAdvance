/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <limits>

#include "timer.hpp"

namespace nba::core {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void Timer::Reset() {
  may_cause_irq = false;
  run_count = 0;
  
  for (int chan_id = 0; chan_id < 4; chan_id++) {
    channels[chan_id].id = chan_id;
    channels[chan_id].cycles = 0;
    channels[chan_id].reload = 0;
    channels[chan_id].counter = 0;
    channels[chan_id].control.frequency = 0;
    channels[chan_id].control.cascade = false;
    channels[chan_id].control.interrupt = false;
    channels[chan_id].control.enable = false;
    channels[chan_id].shift = 0;
    channels[chan_id].mask  = 0;
    channels[chan_id].cascades_into_timer_causing_irq = false;
  }
}

auto Timer::Read(int chan_id, int offset) -> std::uint8_t {
  auto const& channel = channels[chan_id];
  
  switch (offset) {
    case 0: {
      return channel.counter & 0xFF;
    }
    case 1: {
      return channel.counter >> 8;
    }
    case 2: {
      return (channel.control.frequency) |
             (channel.control.cascade   ? 4   : 0) |
             (channel.control.interrupt ? 64  : 0) |
             (channel.control.enable    ? 128 : 0);
    }
    default: return 0;
  }
}

void Timer::Write(int chan_id, int offset, std::uint8_t value) {
  auto& channel = channels[chan_id];
  auto& control = channel.control;
  
  switch (offset) {
    case 0: channel.reload = (channel.reload & 0xFF00) | (value << 0); break;
    case 1: channel.reload = (channel.reload & 0x00FF) | (value << 8); break;
    case 2: {
      bool enable_previous = control.enable;

      control.frequency = value & 3;
      control.interrupt = value & 64;
      control.enable = value & 128;
      if (chan_id != 0) {
        control.cascade = value & 4;
      }
      
      channel.shift = g_ticks_shift[control.frequency];
      channel.mask  = g_ticks_mask[control.frequency];
      channel.samplerate = 16777216 / ((0x10000 - channel.reload) << channel.shift);
      
      if (!enable_previous && control.enable) {
        /* NOTE: the timing calibration test seems to
         * expect the timer to start 2 cycles later than we start it.
         * We account for this here, however I'm not 100% sure if it
         * holds for the general case.
         */
        channel.cycles  = -2;
        channel.counter = channel.reload;
      }
      
      bool connected = false;
      
      may_cause_irq = false;

      /* Identify non-cascading timers that are connected to a
       * cascading timer which is setup to trigger IRQs.
       */
      for (int id = 3; id >= 0; id--) {
        auto const& control = channels[id].control;

        if (!control.enable) {
          connected = false;
          continue;
        }
        
        channels[id].cascades_into_timer_causing_irq = false;
        
        if (connected) {
          connected = control.cascade;
          if (!connected) {
            channels[id].cascades_into_timer_causing_irq = true;
          }
        } else if (control.cascade && control.interrupt) {
          connected = true;
        }
        
        may_cause_irq |= control.interrupt;
      }

      run_count = 0;

      /* Populate list of enabled, clock-driven timers. */
      for (int id = 0; id <= 3; id++) {
        auto const& control = channels[id].control;

        if (control.enable && !control.cascade) {
          run_list[run_count++] = id;
        }
      }
    }
  }
}

void Timer::Run(int cycles) {
  for (int i = 0; i < run_count; i++) {
    auto& channel = channels[run_list[i]];
    int available = channel.cycles + cycles;
      
    Increment(channel.id, available >> channel.shift);
    channel.cycles = available & channel.mask;
  }
}

auto Timer::EstimateCyclesUntilIRQ() -> int {
  int cycles = std::numeric_limits<int>::max();
  
  if (!may_cause_irq) {
    return cycles;
  }
  
  for (int i = 0; i < run_count; i++) {
    auto& channel = channels[run_list[i]];

    if (channel.control.interrupt || channel.cascades_into_timer_causing_irq) {
      int required = ((0x10000 - channel.counter) << channel.shift) - channel.cycles;
      
      if (required < cycles) {
        cycles = required;
      }
    }
  }
  
  return cycles;
}

void Timer::Increment(int chan_id, int increment) {
  auto& channel = channels[chan_id];
  int overflows = 0;
  int threshold = 0x10000 - channel.counter;
  
  if (increment >= threshold) {
    overflows++;
    increment -= threshold;
    channel.counter = channel.reload;
    
    threshold = 0x10000 - channel.reload;
    
    if (increment >= threshold) {
      overflows += increment / threshold;
      increment %= threshold;
    }

    int next_id = chan_id + 1;
    
    if (channel.control.interrupt) {
      irq_controller->Raise(InterruptSource::Timer, chan_id);
    }
    
    if (chan_id <= 1) {
      apu->OnTimerOverflow(chan_id, overflows, channel.samplerate);
    }
    
    if (next_id != 4) {
      auto& control = channels[next_id].control;
      if (control.enable && control.cascade) {
        Increment(next_id, overflows);
      }
    }
  }
      
  channel.counter += increment;
}
  
} // namespace nba::core
