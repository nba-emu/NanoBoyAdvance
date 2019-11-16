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

#include <limits>

#include "cpu.hpp"
#include "timer.hpp"

namespace GameBoyAdvance {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void Timer::Reset() {
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
      }
    }
  }
}

void Timer::Run(int cycles) {
  int available;

  for (auto& channel: channels) {
    if (channel.control.enable && !channel.control.cascade) {
      available = channel.cycles + cycles;
      
      Increment(channel.id, available >> channel.shift);
      channel.cycles = available & channel.mask;
    }
  }
}

auto Timer::EstimateCyclesUntilIRQ() -> int {
  int cycles = std::numeric_limits<int>::max();
  
  /* TODO: optimize and refactor this. */
  for (auto const& channel : channels) {
    if ((channel.control.interrupt || channel.cascades_into_timer_causing_irq) && 
         channel.control.enable &&
        !channel.control.cascade) 
    {
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
  }
      
  channel.counter += increment;
    
  if (overflows > 0) {
    int next_id = chan_id + 1;
    
    if (channel.control.interrupt) {
      cpu->mmio.irq_if |= (CPU::INT_TIMER0 << chan_id);
    }
    
    if (chan_id <= 1) {
      cpu->apu.OnTimerOverflow(chan_id, overflows);
    }
    
    if (next_id != 4 && channels[next_id].control.cascade) {
      Increment(next_id, overflows);
    }
  }
}
  
}