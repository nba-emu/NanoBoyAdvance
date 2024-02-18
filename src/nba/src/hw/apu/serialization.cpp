/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "hw/apu/apu.hpp"

namespace nba::core {

void APU::LoadState(SaveState const& state) {
  mmio.soundcnt.WriteWord(state.apu.io.soundcnt);
  mmio.bias.WriteHalf(state.apu.io.soundbias);

  mmio.psg1.LoadState(state.apu.io.quad[0]);
  mmio.psg2.LoadState(state.apu.io.quad[1]);  
  mmio.psg3.LoadState(state.apu.io.wave);
  mmio.psg4.LoadState(state.apu.io.noise);

  for(int i = 0; i < 2; i++) {
    mmio.fifo[i].LoadState(state.apu.fifo[i]);
    
    fifo_pipe[i].word = state.apu.fifo[i].pipe.word;
    fifo_pipe[i].size = state.apu.fifo[i].pipe.size;
  }

  resolution_old = state.apu.resolution_old;

  // We are simply resetting the MP2K mixer for now,
  // there probably is no need to do complicated (de)serialization.
  mp2k.Reset();
}

void APU::CopyState(SaveState& state) {
  state.apu.io.soundcnt = mmio.soundcnt.ReadWord();
  state.apu.io.soundbias = mmio.bias.ReadHalf();

  mmio.psg1.CopyState(state.apu.io.quad[0]);
  mmio.psg2.CopyState(state.apu.io.quad[1]);  
  mmio.psg3.CopyState(state.apu.io.wave);
  mmio.psg4.CopyState(state.apu.io.noise);

  for(int i = 0; i < 2; i++) {
    mmio.fifo[i].CopyState(state.apu.fifo[i]);

    state.apu.fifo[i].pipe.word = fifo_pipe[i].word;
    state.apu.fifo[i].pipe.size = fifo_pipe[i].size;
  }

  state.apu.resolution_old = resolution_old;
}

void BaseChannel::LoadState(SaveState::APU::IO::PSG const& state) {
  // Sequencer
  enabled = state.enabled;
  step = state.step;

  // Length Counter
  length.enabled = state.length.enabled;
  length.length = state.length.counter; // <- FIX ME

  // Envelope
  envelope.active = state.envelope.active;
  envelope.direction = (Envelope::Direction)state.envelope.direction;
  envelope.initial_volume = state.envelope.initial_volume;
  envelope.current_volume = state.envelope.current_volume;
  envelope.divider = state.envelope.divider;
  envelope.step = state.envelope.step;

  // Sweep
  sweep.active = state.sweep.active;
  sweep.direction = (Sweep::Direction)state.sweep.direction;
  sweep.initial_freq = state.sweep.initial_freq;
  sweep.initial_freq = state.sweep.current_freq;
  sweep.shadow_freq = state.sweep.shadow_freq;
  sweep.divider = state.sweep.divider;
  sweep.shift = state.sweep.shift;
  sweep.step = state.sweep.step;
}

void BaseChannel::CopyState(SaveState::APU::IO::PSG& state) {
  // Sequencer
  state.enabled = enabled;
  state.step = step;

  // Length Counter
  state.length.enabled = length.enabled;
  state.length.counter = length.length; // <- FIX ME

  // Envelope
  state.envelope.active = envelope.active;
  state.envelope.direction = envelope.direction;
  state.envelope.initial_volume = envelope.initial_volume;
  state.envelope.current_volume = envelope.current_volume;
  state.envelope.divider = envelope.divider;
  state.envelope.step = envelope.step;

  // Sweep
  state.sweep.active = sweep.active;
  state.sweep.direction = sweep.direction;
  state.sweep.initial_freq = sweep.initial_freq;
  state.sweep.current_freq = sweep.current_freq;
  state.sweep.shadow_freq = sweep.shadow_freq;
  state.sweep.divider = sweep.divider;
  state.sweep.shift = sweep.shift;
  state.sweep.step = sweep.step;
}

void QuadChannel::LoadState(SaveState::APU::IO::QuadChannel const& state) {
  BaseChannel::LoadState(state);

  dac_enable = state.dac_enable;
  phase = state.phase;
  wave_duty = state.wave_duty;
  sample = state.sample;
  event = scheduler.GetEventByUID(state.event_uid);
}

void QuadChannel::CopyState(SaveState::APU::IO::QuadChannel& state) {
  BaseChannel::CopyState(state);

  state.dac_enable = dac_enable;
  state.phase = phase;
  state.wave_duty = wave_duty;
  state.sample = sample;
  state.event_uid = GetEventUID(event);
}

void WaveChannel::LoadState(SaveState::APU::IO::WaveChannel const& state) {
  BaseChannel::LoadState(state);

  playing = state.playing;
  force_volume = state.force_volume;
  phase = state.phase;
  volume = state.volume;
  frequency = state.frequency;
  dimension = state.dimension;
  wave_bank = state.wave_bank;
  event = scheduler.GetEventByUID(state.event_uid);

  std::memcpy(wave_ram, state.wave_ram, sizeof(wave_ram));
}

void WaveChannel::CopyState(SaveState::APU::IO::WaveChannel& state) {
  BaseChannel::CopyState(state);

  state.playing = playing;
  state.force_volume = force_volume;
  state.phase = phase;
  state.volume = volume;
  state.frequency = frequency;
  state.dimension = dimension;
  state.wave_bank = wave_bank;
  state.event_uid = GetEventUID(event);

  std::memcpy(state.wave_ram, wave_ram, sizeof(wave_ram));
}

void NoiseChannel::LoadState(SaveState::APU::IO::NoiseChannel const& state) {
  BaseChannel::LoadState(state);

  dac_enable = state.dac_enable;
  frequency_shift = state.frequency_shift;
  frequency_ratio = state.frequency_ratio;
  width = state.width;
  event = scheduler.GetEventByUID(state.event_uid);
}

void NoiseChannel::CopyState(SaveState::APU::IO::NoiseChannel& state) {
  BaseChannel::CopyState(state);

  state.dac_enable = dac_enable;
  state.frequency_shift = frequency_shift;
  state.frequency_ratio = frequency_ratio;
  state.width = width;
  state.event_uid = GetEventUID(event);
}

} // namespace nba::core
