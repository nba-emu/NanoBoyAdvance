/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>
#include <limits>

#include "timer.hpp"

namespace nba::core {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void Timer::Reset() {
  for (int chan_id = 0; chan_id < 4; chan_id++) {
    channels[chan_id].id = chan_id;
    channels[chan_id].reload = 0;
    channels[chan_id].counter = 0;
    channels[chan_id].control.frequency = 0;
    channels[chan_id].control.cascade = false;
    channels[chan_id].control.interrupt = false;
    channels[chan_id].control.enable = false;
    channels[chan_id].shift = 0;
    channels[chan_id].mask  = 0;
    channels[chan_id].running = false;
    channels[chan_id].timestamp_started = 0;
    channels[chan_id].event = nullptr;

    channels[chan_id].event_cb = [this, chan_id](int cycles_late) {
      Increment(chan_id, 0x10000 - channels[chan_id].counter);
      StartTimer(chan_id, true, cycles_late);
    };
  }
}

auto Timer::Read(int chan_id, int offset) -> std::uint8_t {
  auto const& channel = channels[chan_id];
  auto const& control = channel.control;

  // The counter value of a cascaded timer can/will only be updated when the
  // connecting timer overflows. But this overflow event may be late by a couple of cycles,
  // unless we explicitly force servicing the event.
  if (channel.control.cascade) {
    scheduler->Step();
  }

  auto counter = channel.counter;

  // While the timer is still running we must account for time that has passed
  // since the last counter update (overflow or configuration change).
  if (channel.running) {
    counter += GetCounterDeltaSinceLastUpdate(chan_id);
  }

  switch (offset) {
    case 0: {
      return counter & 0xFF;
    }
    case 1: {
      return counter >> 8;
    }
    case 2: {
      return (control.frequency) |
             (control.cascade   ? 4   : 0) |
             (control.interrupt ? 64  : 0) |
             (control.enable    ? 128 : 0);
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

      // TODO: only reschedule the timer if something crucial changed?
      // This actually might not be an issue due to the system clock alignment though.
      if (channel.running) {
        StopTimer(chan_id);
      }

      control.frequency = value & 3;
      control.interrupt = value & 64;
      control.enable = value & 128;
      if (chan_id != 0) {
        control.cascade = value & 4;
      }

      channel.shift = g_ticks_shift[control.frequency];
      channel.mask  = g_ticks_mask[control.frequency];
      channel.samplerate = 16777216 / ((0x10000 - channel.reload) << channel.shift);

      if (control.enable) {
        if (!enable_previous) {
          channel.counter = channel.reload;
        }
        if (!control.cascade) {
          // TODO: remove -2 cycle delay if timer already is running?
          StartTimer(chan_id, false, (scheduler->GetTimestampNow() & channel.mask) - 2);
        }
      }
    }
  }
}

auto Timer::GetCounterDeltaSinceLastUpdate(int chan_id) -> std::uint32_t {
  auto const& channel = channels[chan_id];
  return (scheduler->GetTimestampNow() - channel.timestamp_started) >> channel.shift;
}

void Timer::StartTimer(int chan_id, bool force, int cycles_late) {
  auto& channel = channels[chan_id];

  if (channel.running && !force) {
    LOG_ERROR("timer[{0}]: attempted to start timer, but it already is running.", chan_id);
    return;
  }

  int cycles = int((0x10000 - channel.counter) << channel.shift);
  channel.running = true;
  channel.timestamp_started = scheduler->GetTimestampNow() - cycles_late;
  channel.event = scheduler->Add(cycles - cycles_late, channel.event_cb);
}

void Timer::StopTimer(int chan_id) {
  auto& channel = channels[chan_id];

  if (!channel.running) {
    LOG_ERROR("timer[{0}]: attempted to stop timer, but it already is stopped.", chan_id);
    return;
  }

  scheduler->Cancel(channel.event);
  Increment(chan_id, GetCounterDeltaSinceLastUpdate(chan_id));
  channel.event = nullptr;
  channel.running = false;
}

void Timer::Increment(int chan_id, int increment) {
  auto& channel = channels[chan_id];

  channel.counter += increment;

  if (channel.counter >= 0x10000) {
    int overflows = 0;

    /* NOTE: we could assumes that the timer will only overflow a single time,
     * which should generally hold true, except maybe if the reload value is
     * really high (like 0xFFFF). Let's be safe for now.
     */
    do {
      channel.counter = (channel.counter & 0xFFFF) + channel.reload;
      overflows++;
    } while (channel.counter >= 0x10000);

    if (channel.control.interrupt) {
      irq_controller->Raise(InterruptSource::Timer, chan_id);
    }

    if (chan_id <= 1) {
      apu->OnTimerOverflow(chan_id, overflows, channel.samplerate);
    }

    if (chan_id != 3) {
      auto& next_channel = channels[chan_id + 1];
      if (next_channel.control.enable && next_channel.control.cascade) {
        Increment(chan_id + 1, overflows);
      }
    }
  }
}

} // namespace nba::core
