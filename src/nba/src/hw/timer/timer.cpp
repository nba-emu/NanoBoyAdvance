/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/log.hpp>

#include "hw/timer/timer.hpp"

namespace nba::core {

static constexpr int g_ticks_shift[4] = { 0, 6, 8, 10 };
static constexpr int g_ticks_mask[4] = { 0, 0x3F, 0xFF, 0x3FF };

Timer::Timer(Scheduler& scheduler, IRQ& irq, APU& apu)
    : scheduler(scheduler)
    , irq(irq)
    , apu(apu) {
  scheduler.Register(Scheduler::EventClass::TM_overflow, this, &Timer::OnOverflow);
  scheduler.Register(Scheduler::EventClass::TM_write_reload, this, &Timer::OnReloadWritten);
  scheduler.Register(Scheduler::EventClass::TM_write_control, this, &Timer::OnControlWritten);

  Reset();
}

void Timer::Reset() {
  for (int id = 0; id < 4; id++) {
    auto& channel = channels[id];
    channel = {};
    channel.id = id;
  }
}

auto Timer::ReadByte(int chan_id, int offset) -> u8 {
  auto const& channel = channels[chan_id];

  switch (offset) {
    case REG_TMXCNT_L | 0: {
      return (u8)ReadCounter(channel);
    }
    case REG_TMXCNT_L | 1: {
      return (u8)(ReadCounter(channel) >> 8);
    }
    case REG_TMXCNT_H: {
      return ReadControl(channel);
    }
  }

  return 0;
}

auto Timer::ReadHalf(int chan_id, int offset) -> u16 {
  auto const& channel = channels[chan_id];

  switch (offset) {
    case REG_TMXCNT_L: {
      return ReadCounter(channel);
    }
    case REG_TMXCNT_H: {
      return ReadControl(channel);
    }
  }
}

auto Timer::ReadWord(int chan_id) -> u32 {
  auto const& channel = channels[chan_id];

  return (ReadControl(channel) << 16) | ReadCounter(channel);
}

void Timer::WriteByte(int chan_id, int offset, u8 value) {
  auto& channel = channels[chan_id];
  auto& control = channel.control;

  switch (offset) {
    case REG_TMXCNT_L | 0: {
      WriteReload(channel, (channel.reload & 0xFF00) | (value << 0));
      break;
    }
    case REG_TMXCNT_L | 1: {
      WriteReload(channel, (channel.reload & 0x00FF) | (value << 8));
      break;
    }
    case REG_TMXCNT_H: {
      WriteControl(channel, value);
      break;
    }
  }

  if (chan_id <= 1) {
    RecalculateSampleRates();
  }
}

void Timer::WriteHalf(int chan_id, int offset, u16 value) {
  auto& channel = channels[chan_id];
  auto& control = channel.control;

  switch (offset) {
    case REG_TMXCNT_L: {
      WriteReload(channel, value);
      break;
    }
    case REG_TMXCNT_H: {
      WriteControl(channel, value);
      break;
    }
  }

  if (chan_id <= 1) {
    RecalculateSampleRates();
  }
}

void Timer::WriteWord(int chan_id, u32 value) {
  auto& channel = channels[chan_id];

  WriteReload(channel, (u16)value);
  WriteControl(channel, (u32)(value >> 16));

  if (chan_id <= 1) {
    RecalculateSampleRates();
  }
}

auto Timer::ReadCounter(Channel const& channel) -> u16 {
  auto counter = channel.counter;

  // While the timer is still running we must account for time that has passed
  // since the last counter update (overflow or configuration change).
  if (channel.running) {
    counter += GetCounterDeltaSinceLastUpdate(channel);
  }

  return counter;
}

void Timer::WriteReload(Channel& channel, u16 value) {
  channel.pending.reload = value;

  scheduler.Add(1, Scheduler::EventClass::TM_write_reload, 1, channel.id);
}

auto Timer::ReadControl(Channel const& channel) -> u16 {
  auto& control = channel.control;

  return (control.frequency) |
         (control.cascade   ?   4 : 0) |
         (control.interrupt ?  64 : 0) |
         (control.enable    ? 128 : 0);
}

void Timer::WriteControl(Channel& channel, u16 value) {
  channel.pending.control = value;

  scheduler.Add(1, Scheduler::EventClass::TM_write_control, 2, channel.id);
}

void Timer::OnReloadWritten(u64 chan_id) {
  channels[chan_id].reload = channels[chan_id].pending.reload;
}

void Timer::OnControlWritten(u64 chan_id) {
  auto& channel = channels[chan_id];
  auto& control = channel.control;
  bool enable_previous = control.enable;
  u16 value = channel.pending.control;

  if (channel.running) {
    StopChannel(channel);
  }

  control.frequency = value & 3;
  control.interrupt = value & 64;
  control.enable = value & 128;
  if (channel.id != 0) {
    control.cascade = value & 4;
  }

  channel.shift = g_ticks_shift[control.frequency];
  channel.mask = g_ticks_mask[control.frequency];

  if (control.enable) {
    if (!enable_previous) {
      channel.counter = channel.reload;
    }

    if (!control.cascade) {
      auto late = (scheduler.GetTimestampNow() & channel.mask);
      if (!enable_previous) {
        late -= 1;
      }
      StartChannel(channel, late);
    }
  }
}

void Timer::RecalculateSampleRates() {
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

auto Timer::GetCounterDeltaSinceLastUpdate(Channel const& channel) -> u32 {
  return (scheduler.GetTimestampNow() - channel.timestamp_started) >> channel.shift;
}

void Timer::StartChannel(Channel& channel, int cycle_offset) {
  int cycles = int((0x10000 - channel.counter) << channel.shift) - cycle_offset;

  channel.running = true;
  channel.timestamp_started = scheduler.GetTimestampNow() - cycle_offset;
  channel.event_overflow = scheduler.Add(cycles, Scheduler::EventClass::TM_overflow);
}

void Timer::StopChannel(Channel& channel) {
  channel.counter += GetCounterDeltaSinceLastUpdate(channel);
  if (channel.counter >= 0x10000) {
    ReloadCascadeAndRequestIRQ(channel);
  }

  scheduler.Cancel(channel.event_overflow);
  channel.event_overflow = nullptr;
  channel.running = false;
}

void Timer::ReloadCascadeAndRequestIRQ(Channel& channel) {
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
      ReloadCascadeAndRequestIRQ(next_channel);
    }
  }
}

void Timer::OnOverflow(u64 chan_id) {
  auto& channel = channels[chan_id];

  ReloadCascadeAndRequestIRQ(channel);
  StartChannel(channel, 0);
}

} // namespace nba::core
