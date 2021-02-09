/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <common/log.hpp>

#include "timer.hpp"

namespace nba::core {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

void Timer::Reset() {
  for (int id = 0; id < 4; id++) {
    auto& channel = channels[id];
    channel = {};
    channel.id = id;
    channel.event_cb = [this, id](int cycles_late) {
      // FIXME: ideally we would just capture the existing channel reference... not sure if it is possible.
      auto& channel = channels[id];
      OnOverflow(channel);
      StartChannel(channel, cycles_late);
    };
  }
}

auto Timer::Read(int chan_id, int offset) -> std::uint8_t {
  auto const& channel = channels[chan_id];
  auto const& control = channel.control;

  auto counter = channel.counter;

  // While the timer is still running we must account for time that has passed
  // since the last counter update (overflow or configuration change).
  if (channel.running) {
    counter += GetCounterDeltaSinceLastUpdate(channel);
  }

  switch (offset) {
    case REG_TMXCNT_L | 0: {
      return counter & 0xFF;
    }
    case REG_TMXCNT_L | 1: {
      return counter >> 8;
    }
    case REG_TMXCNT_H: {
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
    case REG_TMXCNT_L | 0: channel.reload = (channel.reload & 0xFF00) | (value << 0); break;
    case REG_TMXCNT_L | 1: channel.reload = (channel.reload & 0x00FF) | (value << 8); break;
    case REG_TMXCNT_H: {
      bool enable_previous = control.enable;

      if (channel.running) {
        StopChannel(channel);
      }

      control.frequency = value & 3;
      control.interrupt = value & 64;
      control.enable = value & 128;
      if (chan_id != 0) {
        control.cascade = value & 4;
      }

      channel.shift = g_ticks_shift[control.frequency];
      channel.mask  = g_ticks_mask[control.frequency];

      if (control.enable) {
        if (!enable_previous) {
          channel.counter = channel.reload;
        }
        if (!control.cascade) {
          auto late = (scheduler.GetTimestampNow() & channel.mask);
          // TODO: better understand and emulate this delay.
          if (!enable_previous) {
            late -= 2;
          }
          StartChannel(channel, late);
        }
      }
    }
  }

  if (chan_id <= 1) {
    constexpr int kCyclesPerSecond = 16777216;
    auto timer0_duty = 0x10000 - channels[0].reload;
    auto timer1_duty = 0x10000 - channels[1].reload;
    channels[0].samplerate = kCyclesPerSecond / (timer0_duty << channels[0].shift);
    if (channels[1].control.cascade) {
      channels[1].samplerate = channels[0].samplerate / timer1_duty;
    } else {
      channels[1].samplerate = kCyclesPerSecond / (timer1_duty << channels[1].shift);
    }
  }
}

auto Timer::GetCounterDeltaSinceLastUpdate(Channel const& channel) -> std::uint32_t {
  return (scheduler.GetTimestampNow() - channel.timestamp_started) >> channel.shift;
}

void Timer::StartChannel(Channel& channel, int cycles_late) {
  int cycles = int((0x10000 - channel.counter) << channel.shift);

  channel.running = true;
  channel.timestamp_started = scheduler.GetTimestampNow() - cycles_late;
  channel.event = scheduler.Add(cycles - cycles_late, channel.event_cb);
}

void Timer::StopChannel(Channel& channel) {
  channel.counter += GetCounterDeltaSinceLastUpdate(channel);
  if (channel.counter >= 0x10000) {
    OnOverflow(channel);
  }
  scheduler.Cancel(channel.event);
  channel.event = nullptr;
  channel.running = false;
}

void Timer::OnOverflow(Channel& channel) {
  channel.counter = channel.reload;

  if (channel.control.interrupt) {
    irq.Raise(IRQ::Source::Timer, channel.id);
  }

  if (channel.id <= 1) {
    apu.OnTimerOverflow(channel.id, 1, channel.samplerate);
  }

  if (channel.id != 3) {
    auto& next_channel = channels[channel.id + 1];
    if (next_channel.control.enable && next_channel.control.cascade && ++next_channel.counter == 0x10000) {
      OnOverflow(next_channel);
    }
  }
}

} // namespace nba::core
