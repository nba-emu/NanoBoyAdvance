/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <nba/integer.hpp>

namespace nba {

// TODO: refactor structure to allow for packing.

struct SaveState {
  static constexpr u32 kMagicNumber = 0x5353424E; // NBSS
  static constexpr u32 kCurrentVersion = 4;

  u32 magic;
  u32 version;
  u64 timestamp;

  struct ARM {
    struct RegisterFile {
      u32 gpr[16];
      u32 bank[6][7];
      u32 cpsr;
      u32 spsr[6];
    } regs;

    struct Pipeline {
      u8 access;
      u32 opcode[2];
    } pipe;

    bool irq_line;

    // TODO:
    //bool ldm_usermode_conflict;
    //bool cpu_mode_is_invalid;
    //bool latch_irq_disable;
  } arm;

  struct Bus {
    struct Memory {
      std::array<u8, 0x40000> wram;
      std::array<u8, 0x8000> iram;
      struct Latch {
        u32 bios;
      } latch;
    } memory;

    struct IO {
      // TODO: store the 16-bit register value instead?
      struct WaitstateControl {
        u8 sram;
        u8 ws0[2];
        u8 ws1[2];
        u8 ws2[2];
        u8 phi;
        bool prefetch;
      } waitcnt;

      u8 haltcnt;
      u8 rcnt[2];
      u8 postflg;
    } io;

    struct Prefetch {
      bool active;
      u32 head_address;
      u32 last_address;
      u8 count;
      u8 countdown;
    } prefetch;

    int last_access;
    int parallel_internal_cpu_cycle_limit;
  } bus;

  // TODO: keep track of IRQ delay:
  struct IRQ {
    u8  reg_ime;
    u16 reg_ie;
    u16 reg_if;
  } irq;

  struct PPU {
    struct IO {
      u16 dispcnt;
      u16 dispstat;
      u8 vcount;

      u16 bgcnt[4];
      u16 bghofs[4];
      u16 bgvofs[4];

      struct ReferencePoint {
        s32 initial;
        s32 current;
        bool written;
      } bgx[2], bgy[2];

      s16 bgpa[2];
      s16 bgpb[2];
      s16 bgpc[2];
      s16 bgpd[2];

      u16 winh[2];
      u16 winv[2];
      u16 winin;
      u16 winout;

      struct Mosaic {
        struct {
          u8 size_x;
          u8 size_y;
          u8 counter_y;
        } bg, obj;
      } mosaic;

      u16 bldcnt;
      u16 bldalpha;
      u16 bldy;
    } io;

    bool enable_bg[2][4];
    bool window_scanline_enable[2];
    bool dma3_video_transfer_running;

    u8 pram[0x00400];
    u8 oam [0x00400];
    u8 vram[0x18000];
  } ppu;

  struct APU {
    struct IO {
      struct PSG {
        bool enabled;
        u8 step;
        
        struct Length {
          bool enabled;
          u8 counter;
        } length;

        struct Envelope {
          bool active;
          u8 direction;
          u8 initial_volume;
          u8 current_volume;
          u8 divider;
          u8 step;
        } envelope;

        struct Sweep {
          bool active;
          u8 direction;
          u16 initial_freq;
          u16 current_freq;
          u16 shadow_freq;
          u8 divider;
          u8 shift;
          u8 step;
        } sweep;
      };

      struct QuadChannel : PSG {
        bool dac_enable;
        u8 phase;
        u8 wave_duty;
        s8 sample;
      } quad[2];

      struct WaveChannel : PSG {
        bool playing;
        bool force_volume;
        u8 phase;
        u8 volume;
        u16 frequency;
        u8 dimension;
        u8 wave_bank;
        u8 wave_ram[2][16];
      } wave;

      struct NoiseChannel : PSG {
        bool dac_enable;
        u8 frequency_shift;
        u8 frequency_ratio;
        u8 width;
      } noise;

      u32 soundcnt;
      u16 soundbias;
    } io;

    struct FIFO {
      u32 data[7];
      u32 pending;
      u8 count;

      struct Pipe {
        u32 word;
        u8 size;
      } pipe;
    } fifo[2];

    u8 resolution_old;
    u32 fifo_samplerate[2];
  } apu;

  struct Timer {
    u16 counter;
    u16 reload;
    u16 control;

    struct Pending {
      u16 reload;
      u16 control;
    } pending;
  } timer[4];

  struct DMA {
    struct Channel {
      u32 dst_address;
      u32 src_address;
      u16 length;
      u16 control;

      struct Latch {
        u32 length;
        u32 dst_address;
        u32 src_address;
        u32 bus;
      } latch;

      bool is_fifo_dma;
      u64 enable_event_timestamp;
    } channels[4];

    u8 hblank_set;
    u8 vblank_set;
    u8 video_set;
    u8 runnable_set;
    u32 latch;
  } dma;

  struct Backup {
    u8 data[131072];

    struct FLASH {
      u8 current_bank;
      u8 phase;
      bool enable_chip_id;
      bool enable_erase;
      bool enable_write;
      bool enable_select;
    } flash;

    struct EEPROM {
      u8 state;
      u16 address;
      u64 serial_buffer;
      u8 transmitted_bits;
    } eeprom;
  } backup;

  struct GPIO {
    struct RTC {
      u8 current_bit;
      u8 current_byte;
      u8 reg;
      u8 data;
      u8 buffer[7];

      struct PortData {
        u8 sck;
        u8 sio;
        u8 cs;
      } port;

      u8 state;

      struct ControlRegister {
        bool unknown1;
        bool per_minute_irq;
        bool unknown2;
        bool mode_24h;
        bool poweroff;
      } control;
    } rtc;

    struct SolarSensor {
      bool old_clk;
      u8 counter;
    } solar_sensor;

    bool allow_reads;
    u8 rd_mask;
    u8 port_data;
  } gpio;

  u16 keycnt;
};

} // namespace nba
