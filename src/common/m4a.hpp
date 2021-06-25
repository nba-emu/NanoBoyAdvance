/* Derived from: https://github.com/pret/pokeemerald/blob/b4f83b4f0fcce8c06a6a5a5fd541fb76b1fe9f4c/include/gba/m4a_internal.h */

#pragma once

#include <common/integer.hpp>

static constexpr int kM4AMaxDirectSoundChannels = 12;

using u32ptr = u32;

struct M4ASoundChannel {
  u8 status;
  u8 type;
  u8 rightVolume;
  u8 leftVolume;
  u8 attack;
  u8 decay;
  u8 sustain;
  u8 release;
  u8 ky;
  u8 ev;
  u8 er;
  u8 el;
  u8 echoVolume;
  u8 echoLength;
  u8 d1;
  u8 d2;
  u8 gt;
  u8 mk;
  u8 ve;
  u8 pr;
  u8 rp;
  u8 d3[3];
  u32 ct;
  u32 fw;
  u32 freq;
  u32ptr wav;
  u32 cp;
  u32ptr track;
  u32 pp;
  u32 np;
  u32 d4;
  u16 xpi;
  u16 xpc;
};

struct M4ASoundInfo {
  u32 magic;
  volatile u8 pcmDmaCounter;
  u8 reverb;
  u8 maxChans;
  u8 masterVolume;
  u8 freq;
  u8 mode;
  u8 c15;
  u8 pcmDmaPeriod;
  u8 maxLines;
  u8 gap[3];
  s32 pcmSamplesPerVBlank;
  s32 pcmFreq;
  s32 divFreq;
  u32ptr cgbChans;
  u32 func;
  u32 intp;
  u32ptr FnCgbSound;
  u32ptr FnCgbOscOff;
  u32ptr FnMidiKeyToCgbFreq;
  u32 MPlayJumpTable;
  u32 plynote;
  u32 ExtVolPit;
  u8 gap2[16];
  struct M4ASoundChannel channels[kM4AMaxDirectSoundChannels];
};