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

#include "ppu.hpp"

namespace Core {

    void PPU::IO::DisplayControl::reset() {
        mode              = 0;
        cgb_mode          = false;
        frame_select      = 0;
        hblank_oam_access = false;
        one_dimensional   = false;
        forced_blank      = false;

        for (int i = 0; i < 5; i++) enable[i]     = false;
        for (int i = 0; i < 3; i++) win_enable[i] = false;
    }

    auto PPU::IO::DisplayControl::read(int offset) -> u8 {
        switch (offset) {
            case 0:
                return mode |
                       (cgb_mode          ? 8   : 0) |
                       (frame_select      ? 16  : 0) |
                       (hblank_oam_access ? 32  : 0) |
                       (one_dimensional   ? 64  : 0) |
                       (forced_blank      ? 128 : 0);
            case 1:
                return (enable[0] ? 1  : 0) |
                       (enable[1] ? 2  : 0) |
                       (enable[2] ? 4  : 0) |
                       (enable[3] ? 8  : 0) |
                       (enable[4] ? 16 : 0) |
                       (win_enable[0] ? 32  : 0) |
                       (win_enable[1] ? 64  : 0) |
                       (win_enable[2] ? 128 : 0);
                
            default: return 0;
        }
    }

    void PPU::IO::DisplayControl::write(int offset, u8 value) {
        switch (offset) {
            case 0:
                mode              = value & 7;
                cgb_mode          = value & 8;
                frame_select      = (value >> 4) & 1;
                hblank_oam_access = value & 32;
                one_dimensional   = value & 64;
                forced_blank      = value & 128;
                break;
            case 1:
                enable[0] = value & 1;
                enable[1] = value & 2;
                enable[2] = value & 4;
                enable[3] = value & 8;
                enable[4] = value & 16;
                win_enable[0] = value & 32;
                win_enable[1] = value & 64;
                win_enable[2] = value & 128;
                break;
        }
    }

    void PPU::IO::DisplayStatus::reset() {
        vblank_flag = false;
        hblank_flag = false;
        vcount_flag = false;
        vblank_interrupt = false;
        hblank_interrupt = false;
        vcount_interrupt = false;
        vcount_setting = 0;
    }

    auto PPU::IO::DisplayStatus::read(int offset) -> u8 {
        switch (offset) {
            case 0:
                return (vblank_flag ? 1 : 0) |
                       (hblank_flag ? 2 : 0) |
                       (vcount_flag ? 4 : 0) |
                       (vblank_interrupt ? 8  : 0) |
                       (hblank_interrupt ? 16 : 0) |
                       (vcount_interrupt ? 32 : 0);
            case 1:
                return vcount_setting;
                
            default: return 0;
        }
    }

    void PPU::IO::DisplayStatus::write(int offset, u8 value) {
        switch (offset) {
            case 0:
                vblank_interrupt = value & 8;
                hblank_interrupt = value & 16;
                vcount_interrupt = value & 32;
                break;
            case 1:
                vcount_setting = value;
                break;
        }
    }

    void PPU::IO::BackgroundControl::reset() {
        priority      = 0;
        tile_block    = 0;
        mosaic_enable = false;
        full_palette  = false;
        map_block     = 0;
        wraparound    = false;
        screen_size   = 0;
    }

    auto PPU::IO::BackgroundControl::read(int offset) -> u8 {
        switch (offset) {
            case 0:
                return priority |
                       (tile_block << 2) |
                       (mosaic_enable ? 64 : 0) |
                       (full_palette ? 128 : 0);
            case 1:
                return map_block |
                       (wraparound ? 32 : 0) |
                       (screen_size << 6);
                
            default: return 0;
        }
    }

    void PPU::IO::BackgroundControl::write(int offset, u8 value) {
        switch (offset) {
            case 0:
                priority      = value & 3;
                tile_block    = (value >> 2) & 3;
                mosaic_enable = value & 64;
                full_palette  = value & 128;
                break;
            case 1:
                map_block   = value & 0x1F;
                wraparound  = value & 32;
                screen_size = value >> 6;
                break;
        }
    }

    void PPU::IO::ReferencePoint::reset() {
        value = internal = 0;
    }

    void PPU::IO::ReferencePoint::write(int offset, u8 value) {
        switch (offset) {
            case 0: this->value = (this->value & 0xFFFFFF00) | (value << 0); break;
            case 1: this->value = (this->value & 0xFFFF00FF) | (value << 8); break;
            case 2: this->value = (this->value & 0xFF00FFFF) | (value << 16); break;
            case 3: this->value = (this->value & 0x00FFFFFF) | (value << 24); break;
        }

        internal = PPU::decodeFixed32(this->value);
    }

    void PPU::IO::Mosaic::reset() {
        bg.h  = 0;
        bg.v  = 0;
        obj.h = 0;
        obj.v = 0;
    }

    void PPU::IO::Mosaic::write(int offset, u8 value) {
        switch (offset) {
            case 0: bg.h  = value & 0xF; bg.v  = value >> 4; break;
            case 1: obj.h = value & 0xF; obj.v = value >> 4; break;
        }
    }

    void PPU::IO::BlendControl::reset() {
        sfx = SFX_NONE;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 6; j++) {
                targets[i][j] = false;
            }
        }
    }

    auto PPU::IO::BlendControl::read(int offset) -> u8 {
        switch (offset) {
            case 0:
                return (targets[0][LAYER_BG0] ? 1  : 0) |
                       (targets[0][LAYER_BG1] ? 2  : 0) |
                       (targets[0][LAYER_BG2] ? 4  : 0) |
                       (targets[0][LAYER_BG3] ? 8  : 0) |
                       (targets[0][LAYER_OBJ] ? 16 : 0) |
                       (targets[0][LAYER_BD ] ? 32 : 0) |
                       (sfx << 6);
            case 1:
                return (targets[1][LAYER_BG0] ? 1  : 0) |
                       (targets[1][LAYER_BG1] ? 2  : 0) |
                       (targets[1][LAYER_BG2] ? 4  : 0) |
                       (targets[1][LAYER_BG3] ? 8  : 0) |
                       (targets[1][LAYER_OBJ] ? 16 : 0) |
                       (targets[1][LAYER_BD ] ? 32 : 0);
                
            default: return 0;
        }
    }

    void PPU::IO::WindowRange::reset() {
        min = max = 0;
    }

    void PPU::IO::WindowRange::write(int offset, u8 value) {
        int min_old = min;
        int max_old = max;

        switch (offset) {
            case 0: max = value & 0xFF; break;
            case 1: min = value & 0xFF; break;
        }
        if (min != min_old || max != max_old) {
            changed = true;
        }
    }

    void PPU::IO::WindowLayerSelect::reset() {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 6; j++) {
                enable[i][j] = false;
            }
        }
    }

    auto PPU::IO::WindowLayerSelect::read(int offset) -> u8 {
        return (enable[offset][LAYER_BG0] ? 1  : 0) |
               (enable[offset][LAYER_BG1] ? 2  : 0) |
               (enable[offset][LAYER_BG2] ? 4  : 0) |
               (enable[offset][LAYER_BG3] ? 8  : 0) |
               (enable[offset][LAYER_OBJ] ? 16 : 0) |
               (enable[offset][LAYER_SFX] ? 32 : 0);
    }

    void PPU::IO::WindowLayerSelect::write(int offset, u8 value) {
        enable[offset][LAYER_BG0] = value & 1;
        enable[offset][LAYER_BG1] = value & 2;
        enable[offset][LAYER_BG2] = value & 4;
        enable[offset][LAYER_BG3] = value & 8;
        enable[offset][LAYER_OBJ] = value & 16;
        enable[offset][LAYER_SFX] = value & 32;
    }

    void PPU::IO::BlendControl::write(int offset, u8 value) {
        switch (offset) {
            case 0:
                targets[0][LAYER_BG0] = value & 1;
                targets[0][LAYER_BG1] = value & 2;
                targets[0][LAYER_BG2] = value & 4;
                targets[0][LAYER_BG3] = value & 8;
                targets[0][LAYER_OBJ] = value & 16;
                targets[0][LAYER_BD ] = value & 32;
                sfx = static_cast<SpecialEffect>(value >> 6);
                break;
            case 1:
                targets[1][LAYER_BG0] = value & 1;
                targets[1][LAYER_BG1] = value & 2;
                targets[1][LAYER_BG2] = value & 4;
                targets[1][LAYER_BG3] = value & 8;
                targets[1][LAYER_OBJ] = value & 16;
                targets[1][LAYER_BD ] = value & 32;
                break;
        }
    }

    void PPU::IO::BlendAlpha::reset() {
        eva = evb = 0;
    }

    void PPU::IO::BlendAlpha::write(int offset, u8 value) {
        switch (offset) {
            case 0: eva = value & 0x1F; break;
            case 1: evb = value & 0x1F; break;
        }
    }

    void PPU::IO::BlendY::reset() {
        evy = 0;
    }

    void PPU::IO::BlendY::write(u8 value) {
        evy = value & 0x1F;
    }
}
