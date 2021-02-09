/* Derived from: https://github.com/pret/pokeemerald/blob/b4f83b4f0fcce8c06a6a5a5fd541fb76b1fe9f4c/include/gba/m4a_internal.h */

#pragma once

#include <cstdint>

static constexpr int kM4AMaxDirectSoundChannels = 12;

using u32ptr_t = std::uint32_t;

/* This is really poorly made and I hope it doesn't fall into pieces... */

struct M4ASoundChannel {
  std::uint8_t status;
  std::uint8_t type;
  std::uint8_t rightVolume;
  std::uint8_t leftVolume;
  std::uint8_t attack;
  std::uint8_t decay;
  std::uint8_t sustain;
  std::uint8_t release;
  std::uint8_t ky;
  std::uint8_t ev;
  std::uint8_t er;
  std::uint8_t el;
  std::uint8_t echoVolume;
  std::uint8_t echoLength;
  std::uint8_t d1;
  std::uint8_t d2;
  std::uint8_t gt;
  std::uint8_t mk;
  std::uint8_t ve;
  std::uint8_t pr;
  std::uint8_t rp;
  std::uint8_t d3[3];
  std::uint32_t ct;
  std::uint32_t fw;
  std::uint32_t freq;
  u32ptr_t wav;
  std::uint32_t cp;
  u32ptr_t track;
  std::uint32_t pp;
  std::uint32_t np;
  std::uint32_t d4;
  std::uint16_t xpi;
  std::uint16_t xpc;
};

struct M4ASoundInfo {
  std::uint32_t magic;
  volatile std::uint8_t pcmDmaCounter;
  std::uint8_t reverb;
  std::uint8_t maxChans;
  std::uint8_t masterVolume;
  std::uint8_t freq;
  std::uint8_t mode;
  std::uint8_t c15;
  std::uint8_t pcmDmaPeriod;
  std::uint8_t maxLines;
  std::uint8_t gap[3];
  std::int32_t pcmSamplesPerVBlank;
  std::int32_t pcmFreq;
  std::int32_t divFreq;
  u32ptr_t cgbChans;
  std::uint32_t func;
  std::uint32_t intp;
  u32ptr_t FnCgbSound;
  u32ptr_t FnCgbOscOff;
  u32ptr_t FnMidiKeyToCgbFreq;
  std::uint32_t MPlayJumpTable;
  std::uint32_t plynote;
  std::uint32_t ExtVolPit;
  std::uint8_t gap2[16];
  struct M4ASoundChannel channels[kM4AMaxDirectSoundChannels];
};