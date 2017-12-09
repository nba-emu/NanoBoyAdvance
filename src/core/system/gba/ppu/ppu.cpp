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

#include <cmath>
#include <algorithm>
#include "ppu.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace Core {

    PPU::PPU(Config* config, u8* paletteMemory, u8* objectMemory, u8* videoMemory) :
            m_config (config),
            m_pal    (paletteMemory),
            m_oam    (objectMemory),
            m_vram   (videoMemory)
    {
        reset();
        reloadConfig();

        // Generate blending LUT (TODO: make table static)
        for (int color0 = 0; color0 <= 31; color0++)
        for (int color1 = 0; color1 <= 31; color1++)
        for (int factor0 = 0; factor0 <= 16; factor0++)
        for (int factor1 = 0; factor1 <= 16; factor1++) {
            int result = (color0 * factor0 + color1 * factor1) >> 4;
            blend_table[factor0][factor1][color0][color1] = std::min<u8>(result, 31);
        }
    }

    void PPU::reloadConfig() {
        m_frameskip   = m_config->frameskip;
        m_framebuffer = m_config->framebuffer;

        // build color LUT
        for (int color = 0; color < 0x8000; color++) {
            if (m_config->darken_screen) {
                double r = ((color >> 0 ) & 0x1F) / 31.0;
                double g = ((color >> 5 ) & 0x1F) / 31.0;
                double b = ((color >> 10) & 0x1F) / 31.0;

                r = std::pow(r, 4.0) * 48;
                g = std::pow(g, 3.0) * 48;
                b = std::pow(b, 1.4) * 48;

                m_color_lut[color] = 0xFF000000     |
                                     ((int)b << 0 ) |
                                     ((int)g << 8 ) |
                                     ((int)r << 16);
            } else {
                int r = (color >> 0 ) & 0x1F;
                int g = (color >> 5 ) & 0x1F;
                int b = (color >> 10) & 0x1F;

                m_color_lut[color] = 0xFF000000 | (b << 3) | (g << 11) | (r << 19);
            }
        }
    }

    void PPU::reset() {
        regs.vcount = 0;

        // reset DISPCNT and DISPCNT
        regs.control.reset();
        regs.status.reset();

        // reset all backgrounds
        for (int i = 0; i < 4; i++) {
            regs.bghofs[i] = 0;
            regs.bgvofs[i] = 0;
            regs.bgcnt[i].reset();

            if (i < 2) {
                regs.bgx[i].reset();
                regs.bgy[i].reset();
                regs.bgpa[i] = 0;
                regs.bgpb[i] = 0;
                regs.bgpc[i] = 0;
                regs.bgpd[i] = 0;
            }
        }

        // reset MOSAIC register
        regs.mosaic.reset();

        // reset blending registers
        regs.bldcnt.reset();
        regs.bldalpha.reset();
        regs.bldy.reset();

        // reset window registers
        regs.winin.reset();
        regs.winout.reset();
        regs.winh[0].reset();
        regs.winv[0].reset();
        regs.winh[1].reset();
        regs.winv[1].reset();

        m_frame_counter = 0;
        line_has_alpha_objs = false;
    }

    void PPU::setMemoryBuffers(u8* pal, u8* oam, u8* vram) {
        m_pal  = pal;
        m_oam  = oam;
        m_vram = vram;
    }

    void PPU::setInterruptController(Interrupt* interrupt) {
        m_interrupt = interrupt;
    }

    void PPU::hblank() {
        regs.status.vblank_flag = false;
        regs.status.hblank_flag = true;

        if (regs.status.hblank_interrupt)
            m_interrupt->request(INTERRUPT_HBLANK);
    }

    void PPU::vblank() {
        regs.status.vblank_flag = true;
        regs.status.hblank_flag = false;

        regs.bgx[0].internal = PPU::decodeFixed32(regs.bgx[0].value);
        regs.bgy[0].internal = PPU::decodeFixed32(regs.bgy[0].value);
        regs.bgx[1].internal = PPU::decodeFixed32(regs.bgx[1].value);
        regs.bgy[1].internal = PPU::decodeFixed32(regs.bgy[1].value);

        if (m_config->frameskip != 0) {
            m_frame_counter = (m_frame_counter + 1) % m_config->frameskip;
        }

        if (regs.status.vblank_interrupt) {
            m_interrupt->request(INTERRUPT_VBLANK);
        }
    }

    void PPU::scanline(bool render) {
        // TODO: maybe find a better way
        regs.status.vblank_flag = false;
        regs.status.hblank_flag = false;

        // Are we off by one scanline?
        regs.bgx[0].internal += PPU::decodeFixed16(regs.bgpb[0]);
        regs.bgy[0].internal += PPU::decodeFixed16(regs.bgpd[0]);
        regs.bgx[1].internal += PPU::decodeFixed16(regs.bgpb[1]);
        regs.bgy[1].internal += PPU::decodeFixed16(regs.bgpd[1]);

        if (render && (m_frameskip == 0 || m_frame_counter == 0)) {
            u16 backdrop_color = *(u16*)(m_pal); // TODO: endianess
            u32* line_buffer   = &m_framebuffer[regs.vcount * 240];

            // Simulate forced blank and bail out early
            if (regs.control.forced_blank) {
                u32* line = m_framebuffer + regs.vcount * 240;

                for (int x = 0; x < 240; x++) {
                    line[x] = rgb555ToARGB(0x7FFF);
                }
                return;
            }

            // Render window masks
            if (regs.control.win_enable[0]) {
                renderWindow(0);
            }
            if (regs.control.win_enable[1]) {
                renderWindow(1);
            }

            #define DECLARE_VARS_NO_SFX \
                int current_prio = 4;\
                u16 out_pixel    = backdrop_color;

            #define IS_SUITABLE_BG_NO_SFX (enable[bg] && bgcnt[bg].priority <= current_prio)

            #define SUITABLE_BG_NO_SFX_INNER \
                u16 pixel;\
                pixel = m_buffer[bg][x];\
                if (pixel != COLOR_TRANSPARENT) {\
                    out_pixel    = pixel;\
                    current_prio = bgcnt[bg].priority;\
                }

            #define IS_OBJ_PIXEL_NO_SFX (enable[LAYER_OBJ] && m_obj_layer[x].prio <= current_prio)

            #define OBJ_PIXEL_NO_SFX_INNER \
                u16 pixel = m_obj_layer[x].pixel;\
                if (pixel != COLOR_TRANSPARENT) {\
                    line_buffer[x] = rgb555ToARGB(pixel);\
                    continue;\
                }

            #define DECLARE_SFX_VARS \
                int layer[2] = { LAYER_BD, LAYER_BD };\
                u16 pixel[2] = { backdrop_color, 0  };

            #define IS_SUITABLE_BG_SFX (enable[bg] && bgcnt[bg].priority == prio)

            #define SUITABLE_BG_SFX_INNER \
                u16 new_pixel = m_buffer[bg][x];\
                if (new_pixel != COLOR_TRANSPARENT) {\
                    layer[1] = layer[0];\
                    layer[0] = bg;\
                    pixel[1] = pixel[0];\
                    pixel[0] = new_pixel;\
                }

            #define IS_OBJ_PIXEL_SFX (enable[LAYER_OBJ] && m_obj_layer[x].prio == prio)

            #define OBJ_PIXEL_SFX_INNER \
                u16 new_pixel = m_obj_layer[x].pixel;\
                if (new_pixel != COLOR_TRANSPARENT) {\
                    layer[1] = layer[0];\
                    layer[0] = LAYER_OBJ;\
                    pixel[1] = pixel[0];\
                    pixel[0] = new_pixel;\
                }

            #define PERFORM_SFX_EFFECT \
                auto sfx          = regs.bldcnt.sfx;\
                bool is_alpha_obj = layer[0] == LAYER_OBJ && m_obj_layer[x].alpha;\
                bool sfx_above    = regs.bldcnt.targets[0][layer[0]] || is_alpha_obj;\
                bool sfx_below    = regs.bldcnt.targets[1][layer[1]];\
                \
                if (is_alpha_obj && sfx_below) {\
                    sfx = SFX_BLEND;\
                }\
                \
                if (sfx != SFX_NONE && sfx_above && (sfx_below || sfx != SFX_BLEND)) {\
                    blendPixels(&pixel[0], pixel[1], sfx);\
                }

            #define DECLARE_WIN_VARS \
                const auto& outside  = regs.winout.enable[0];\
                const auto& win0in   = regs.winin.enable[0];\
                const auto& win1in   = regs.winin.enable[1];\
                const auto& objwinin = regs.winout.enable[1];\

            #define DECLARE_WIN_VARS_INNER \
                const bool win0   = win_enable[0] && m_win_scanline_enable[0] && m_win_mask[0][x];\
                const bool win1   = win_enable[1] && m_win_scanline_enable[1] && m_win_mask[1][x];\
                const bool objwin = win_enable[2] && m_obj_layer[x].window;\
                \
                const bool* visible = win0   ? win0in   :\
                                      win1   ? win1in   :\
                                      objwin ? objwinin : outside;

            #define  BG_IS_IN_WINDOW visible[bg]
            #define OBJ_IS_IN_WINDOW visible[LAYER_OBJ]

            #define LOOP_LINE \
                for (int x = 0; x < 240; x++)

            #define LOOP_PRIO \
                for (int prio = 3; prio >= 0; prio--)

            #define LOOP_BG_MODE_0 \
                for (int bg = 3; bg >= 0; bg--)

            #define LOOP_BG_MODE_1 \
                for (int bg = 2; bg >= 0; bg--)

            #define LOOP_BG_MODE_2 \
                for (int bg = 3; bg >= 2; bg--)

            #define LOOP_BG_MODE_3_4_5 \
                int bg = 2;

            const auto& bgcnt  = regs.bgcnt;
            const auto& enable = regs.control.enable;

            auto win_enable = regs.control.win_enable;
            bool no_windows = !win_enable[0] && !win_enable[1] && !win_enable[2];
            bool no_effects = (regs.bldcnt.sfx == SFX_NONE) && !line_has_alpha_objs;

            #define COMPOSE(custom_bg_loop) \
                if (no_windows && no_effects) {\
                    LOOP_LINE {\
                        DECLARE_VARS_NO_SFX;\
                        custom_bg_loop {\
                            if (IS_SUITABLE_BG_NO_SFX) {\
                                SUITABLE_BG_NO_SFX_INNER;\
                            }\
                        }\
                        if (IS_OBJ_PIXEL_NO_SFX) {\
                            OBJ_PIXEL_NO_SFX_INNER;\
                        }\
                        line_buffer[x] = rgb555ToARGB(out_pixel);\
                    }\
                }\
                else if (!no_windows &&  no_effects) {\
                    DECLARE_WIN_VARS;\
                    DECLARE_VARS_NO_SFX;\
                    LOOP_LINE {\
                        DECLARE_VARS_NO_SFX;\
                        DECLARE_WIN_VARS_INNER\
                        custom_bg_loop {\
                            if (IS_SUITABLE_BG_NO_SFX && BG_IS_IN_WINDOW) {\
                                SUITABLE_BG_NO_SFX_INNER;\
                            }\
                        }\
                        if (IS_OBJ_PIXEL_NO_SFX && OBJ_IS_IN_WINDOW) {\
                            OBJ_PIXEL_NO_SFX_INNER;\
                        }\
                        line_buffer[x] = rgb555ToARGB(out_pixel);\
                    }\
                }\
                else if ( no_windows && !no_effects) {\
                    DECLARE_WIN_VARS;\
                    LOOP_LINE {\
                        DECLARE_SFX_VARS;\
                        DECLARE_WIN_VARS_INNER;\
                        LOOP_PRIO {\
                            custom_bg_loop {\
                                if (IS_SUITABLE_BG_SFX) {\
                                    SUITABLE_BG_SFX_INNER;\
                                }\
                            }\
                            if (IS_OBJ_PIXEL_SFX) {\
                                OBJ_PIXEL_SFX_INNER;\
                            }\
                        }\
                        PERFORM_SFX_EFFECT;\
                        line_buffer[x] = rgb555ToARGB(pixel[0]);\
                    }\
                }\
                else {\
                    DECLARE_WIN_VARS;\
                    LOOP_LINE {\
                        DECLARE_SFX_VARS;\
                        DECLARE_WIN_VARS_INNER;\
                        LOOP_PRIO {\
                            custom_bg_loop {\
                                if (IS_SUITABLE_BG_SFX && BG_IS_IN_WINDOW) {\
                                    SUITABLE_BG_SFX_INNER;\
                                }\
                            }\
                            if (IS_OBJ_PIXEL_SFX && OBJ_IS_IN_WINDOW) {\
                                OBJ_PIXEL_SFX_INNER;\
                            }\
                        }\
                        if (visible[LAYER_SFX]) {\
                            PERFORM_SFX_EFFECT;\
                        }\
                        line_buffer[x] = rgb555ToARGB(pixel[0]);\
                    }\
                }

            switch (regs.control.mode) {
                case 0: {
                    // BG Mode 0 - 240x160 pixels, Text mode
                    if (regs.control.enable[0]) renderTextBG(0);
                    if (regs.control.enable[1]) renderTextBG(1);
                    if (regs.control.enable[2]) renderTextBG(2);
                    if (regs.control.enable[3]) renderTextBG(3);
                    if (regs.control.enable[4]) renderSprites();
                    COMPOSE(LOOP_BG_MODE_0);
                    break;
                }
                case 1:
                    // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
                    if (regs.control.enable[0]) renderTextBG(0);
                    if (regs.control.enable[1]) renderTextBG(1);
                    if (regs.control.enable[2]) renderAffineBG(0);
                    if (regs.control.enable[4]) renderSprites();
                    COMPOSE(LOOP_BG_MODE_1);
                    break;
                case 2:
                    // BG Mode 2 - 240x160 pixels, RS mode
                    if (regs.control.enable[2]) renderAffineBG(0);
                    if (regs.control.enable[3]) renderAffineBG(1);
                    if (regs.control.enable[4]) renderSprites();
                    COMPOSE(LOOP_BG_MODE_2);
                    break;
                case 3:
                    // BG Mode 3 - 240x160 pixels, 32768 colors
                    if (regs.control.enable[2]) {
                        renderBitmapMode1BG();
                    }
                    COMPOSE(LOOP_BG_MODE_3_4_5);
                    break;
                case 4:
                    // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
                    if (regs.control.enable[2]) {
                        renderBitmapMode2BG();
                    }
                    if (regs.control.enable[4]) {
                        renderSprites();
                    }
                    COMPOSE(LOOP_BG_MODE_3_4_5);
                    break;
                case 5:
                    // BG Mode 5 - 160x128 pixels, 32768 colors
                    if (regs.control.enable[2]) {
                        renderBitmapMode3BG();
                    }
                    COMPOSE(LOOP_BG_MODE_3_4_5);
                    break;
            }
        }
    }

    void PPU::nextLine() {
        bool vcount_flag = regs.vcount == regs.status.vcount_setting;

        regs.vcount = (regs.vcount + 1) % 228;
        regs.status.vcount_flag = vcount_flag;

        if (vcount_flag && regs.status.vcount_interrupt) {
            m_interrupt->request(INTERRUPT_VCOUNT);
        }
    }

    void PPU::renderWindow(int id) {
        int   line = regs.vcount;
        auto& winv = regs.winv[id];

        // Check if the current scanline is outside of the window.
        if ((winv.min <= winv.max && (line < winv.min || line >= winv.max)) ||
            (winv.min >  winv.max && (line < winv.min && line >= winv.max)))
        {
            // Mark window as inactive during the current scanline.
            m_win_scanline_enable[id] = false;
        } else {
            auto& winh = regs.winh[id];

            // Mark window as active during the current scanline.
            m_win_scanline_enable[id] = true;

            // Only recalculate the LUTs if min/max changed between the last update & now.
            if (winh.changed) {
                if (winh.min <= winh.max) {
                    for (int x = 0; x < 240; x++) {
                        m_win_mask[id][x] = x >= winh.min && x < winh.max;
                    }
                } else {
                    for (int x = 0; x < 240; x++) {
                        m_win_mask[id][x] = x >= winh.min || x < winh.max;
                    }
                }
                winh.changed = false;
            }
        }
    }

    void PPU::blendPixels(u16* target1, u16 target2, SpecialEffect sfx) {
        int r1 = (*target1 >> 0 ) & 0x1F;
        int g1 = (*target1 >> 5 ) & 0x1F;
        int b1 = (*target1 >> 10) & 0x1F;

        switch (sfx) {
            case SFX_BLEND: {
                int eva = std::min<int>(16, regs.bldalpha.eva);
                int evb = std::min<int>(16, regs.bldalpha.evb);

                int r2 = (target2 >> 0 ) & 0x1F;
                int g2 = (target2 >> 5 ) & 0x1F;
                int b2 = (target2 >> 10) & 0x1F;

                r1 = blend_table[eva][evb][r1][r2];
                g1 = blend_table[eva][evb][g1][g2];
                b1 = blend_table[eva][evb][b1][b2];
                break;
            }
            case SFX_INCREASE: {
                int evy = std::min<int>(16, regs.bldy.evy);

                r1 = blend_table[16 - evy][evy][r1][31];
                g1 = blend_table[16 - evy][evy][g1][31];
                b1 = blend_table[16 - evy][evy][b1][31];
                break;
            }
            case SFX_DECREASE: {
                int evy = std::min<int>(16, regs.bldy.evy);

                r1 = blend_table[16 - evy][evy][r1][0];
                g1 = blend_table[16 - evy][evy][g1][0];
                b1 = blend_table[16 - evy][evy][b1][0];
                break;
            }
            default: break;
        }

        *target1 = (r1 << 0 ) | (g1 << 5 ) | (b1 << 10);
    }
}
