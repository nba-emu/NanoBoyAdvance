/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
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

#include "apu.hpp"

namespace GameBoyAdvance {
    
    void APU::IO::ToneChannel::reset() {
        
        // reset sweep info
        sweep.time = 0;
        sweep.shift = 0;
        sweep.direction = SWEEP_INC;
        
        // reset envelope info
        envelope.time = 0;
        envelope.initial = 0;
        envelope.direction = ENV_DEC;
        
        frequency = 0;
        wave_duty = 0;
        sound_length = 0;
        apply_length = false;
        
        // reset internal state
        internal.sample    = 0;
        internal.volume    = 0;
        internal.frequency = 0;
        internal.cycles.sweep    = 0;
        internal.cycles.length   = 0;
        internal.cycles.envelope = 0;
    }
    
    auto APU::IO::ToneChannel::read(int offset) -> u8 {
        
        switch (offset) {
            // Sweep Register
            case 0: {
                return (sweep.shift     << 0) |
                       (sweep.direction << 3) |
                       (sweep.time      << 4);
            }
            case 1: { return 0; }
                
            // Duty/Len/Envelope
            case 2: {
                return /*(sound_length << 0) |*/
                       (wave_duty    << 6);
            }
            case 3: {
                return (envelope.time      << 0) |
                       (envelope.direction << 3) |
                       (envelope.initial   << 4);
            }
                
            // Frequency/Control
            case 4: { return 0; }
            case 5: {
                return apply_length ? 0x40 : 0;
            }
        }
    }
    
    void APU::IO::ToneChannel::write(int offset, u8 value) {
        
        switch (offset) {
            // Sweep Register
            case 0: {
                sweep.shift     = (value >> 0) & 3;
                sweep.direction = (value >> 3) & 1;
                sweep.time      = (value >> 4);
                break;
            }
            case 1: { break; }
                
            // Duty/Len/Envelope
            case 2: {
                sound_length = (value >> 0) & 63;
                wave_duty    = (value >> 6) & 3;
                break;
            }
            case 3: {
                envelope.time      = (value >> 0) & 7;
                //envelope.direction = (value >> 3) & 1;
                envelope.initial   = (value >> 4);
                break;
            }
                
            // Frequency Control
            case 4: {
                frequency = (frequency & ~0xFF) | value;
                break;
            }
            case 5: {
                frequency = (frequency &  0xFF) | ((value & 7) << 8);
                apply_length = value & 0x40;
                
                // on sound restart
                if (value & 0x80) {
                    // reload initial volume and frequency
                    internal.volume    = envelope.initial;
                    internal.frequency = frequency;
                    
                    // reset cycle counters
                    internal.cycles.sweep    = 0;
                    internal.cycles.length   = 0;
                    internal.cycles.envelope = 0;
                }
                break;
            }
        }
    }
    
    void APU::IO::WaveChannel::reset() {
        playback     = false;
        force_volume = false;
        apply_length = false;
        
        volume       = 0;
        frequency    = 0;
        dimension    = 0;
        bank_number  = 0;
        sound_length = 0;
        
        length_cycles = 0;
    }
    
    auto APU::IO::WaveChannel::read(int offset) -> u8 {
        switch (offset) {
            // Stop/Wave RAM select
            case 0: {
                return (dimension   << 5       ) |
                       (bank_number << 6       ) |
                       (playback     ? 0x80 : 0);
            }
            case 1: { return 0; }
                
            // Length/Volume
            case 2: { return 0; }
            case 3: {
                return (volume       << 5       ) |
                       (force_volume  ? 0x80 : 0);
            }
                
            // Frequency/Control
            case 4: { return 0; }
            case 5: {
                return apply_length ? 0x40 : 0;
            }
        }
    }
    
    void APU::IO::WaveChannel::write(int offset, u8 value) {
        switch (offset) {
            // Stop/Wave RAM select
            case 0: {
                dimension   = (value >> 5) & 1;
                bank_number = (value >> 6) & 1;
                playback    =  value & 0x80;
                break;
            }
            case 1: { break; }
            
            // Length/Volume
            case 2: {
                sound_length = value;
                break;
            }
            case 3: {
                volume       = (value >> 5) & 3;
                force_volume = value & 0x80;
                break;
            }
                
            // Frequency/Control
            case 4: {
                frequency = (frequency & ~0xFF) | value;
                break;
            }
            case 5: {
                frequency    = (frequency & 0xFF) | ((value & 7) << 8);
                apply_length = value & 0x40;
                
                // on sound restart
                if (value & 0x80) {
                    length_cycles = 0;
                }
                break;
            }
        }
    }
    
    void APU::IO::Control::reset() {
        master_enable = false;
        psg.volume = 0;
        psg.master[0] = 0;
        psg.master[1] = 0;
        psg.enable[0][0] = false;
        psg.enable[0][1] = false;
        psg.enable[0][2] = false;
        psg.enable[0][3] = false;
        psg.enable[1][0] = false;
        psg.enable[1][1] = false;
        psg.enable[1][2] = false;
        psg.enable[1][3] = false;
        dma[0].volume = 0;
        dma[0].enable[0] = false;
        dma[0].enable[1] = false;
        dma[0].timer_num = 0;
        dma[1].volume = 0;
        dma[1].enable[0] = false;
        dma[1].enable[1] = false;
        dma[1].timer_num = 0;
    }
    
    auto APU::IO::Control::read(int offset) -> u8 {
        switch (offset) {
            case 0:
                return (psg.master[SIDE_RIGHT] << 0) |
                       (psg.master[SIDE_LEFT]  << 4);
            case 1:
                return (psg.enable[SIDE_RIGHT][0] ? 1   : 0) |
                       (psg.enable[SIDE_RIGHT][1] ? 2   : 0) |
                       (psg.enable[SIDE_RIGHT][2] ? 4   : 0) |
                       (psg.enable[SIDE_RIGHT][3] ? 8   : 0) |
                       (psg.enable[SIDE_LEFT ][0] ? 16  : 0) |
                       (psg.enable[SIDE_LEFT ][1] ? 32  : 0) |
                       (psg.enable[SIDE_LEFT ][2] ? 64  : 0) |
                       (psg.enable[SIDE_LEFT ][3] ? 128 : 0);
            case 2:
                return (psg.volume            ) |
                       (dma[DMA_A].volume << 2) |
                       (dma[DMA_B].volume << 3);
            case 3:
                return (dma[DMA_A].enable[SIDE_RIGHT] ? 1  : 0) |
                       (dma[DMA_A].enable[SIDE_LEFT ] ? 2  : 0) |
                       (dma[DMA_A].timer_num          ? 4  : 0) |
                       (dma[DMA_B].enable[SIDE_RIGHT] ? 16 : 0) |
                       (dma[DMA_B].enable[SIDE_LEFT ] ? 32 : 0) |
                       (dma[DMA_B].timer_num          ? 64 : 0);
            case 4:
                // TODO(accuracy): actually emulate bits 0-3
                return 0b1111 | (master_enable ? 128 : 0);
        }
    }
    
    void APU::IO::Control::write(int offset, u8 value) {
        switch (offset) {
            case 0:
                psg.master[SIDE_RIGHT] = (value >> 0) & 7;
                psg.master[SIDE_LEFT ] = (value >> 4) & 7;
                break;
            case 1:
                psg.enable[SIDE_RIGHT][0] = value & 1;
                psg.enable[SIDE_RIGHT][1] = value & 2;
                psg.enable[SIDE_RIGHT][2] = value & 4;
                psg.enable[SIDE_RIGHT][3] = value & 8;
                psg.enable[SIDE_LEFT ][0] = value & 16;
                psg.enable[SIDE_LEFT ][1] = value & 32;
                psg.enable[SIDE_LEFT ][2] = value & 64;
                psg.enable[SIDE_LEFT ][3] = value & 128;
                break;
            case 2:
                psg.volume        = (value >> 0) & 3;
                dma[DMA_A].volume = (value >> 2) & 1;
                dma[DMA_B].volume = (value >> 3) & 1;
                break;
            case 3:
                dma[DMA_A].enable[SIDE_RIGHT] = value & 1;
                dma[DMA_A].enable[SIDE_LEFT ] = value & 2;
                dma[DMA_A].timer_num          = (value >> 2) & 1;
                dma[DMA_B].enable[SIDE_RIGHT] = value & 16;
                dma[DMA_B].enable[SIDE_LEFT ] = value & 32;
                dma[DMA_B].timer_num          = (value >> 6) & 1;
                
                if (value & 0x08) {
                    fifo[0]->reset();
                }
                if (value & 0x80) {
                    fifo[1]->reset();
                }
                
                break;
            case 4:
                master_enable = value & 128;
                break;
        }
    }
    
    void APU::IO::BIAS::reset() {
        level      = 0x200;
        resolution = 0;
    }
    
    auto APU::IO::BIAS::read(int offset) -> u8 {
        switch (offset) {
            case 0: return level & 0xFF;
            case 1: return ((level >> 8) & 3) | (resolution << 6);
        }
    }
    
    void APU::IO::BIAS::write(int offset, u8 value) {
        switch (offset) {
            case 0: 
                level = (level & ~0xFF) | value;
                break;
            case 1: 
                level      = (level & 0xFF) | (value << 8);
                resolution = value >> 6;
                break;
        }
    }
}