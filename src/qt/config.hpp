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

#pragma once

#include "util/ini.hpp"
#include "core/system/gba/config.hpp"

struct QtConfig : public Core::Config { 
    struct Video {
        int  scale;
        bool keep_ar;
    } video;

    struct Keymap {

    } keymap;

    static auto fromINI(Util::INI* ini) -> QtConfig* {
        auto config = new QtConfig();
        
        QtConfig::fromINI(ini, config);

        return config;
    }

    static void fromINI(Util::INI* ini, QtConfig* config) {
        config->ini = ini;
        config->reload();
    }

    void reload() {
        if (ini == nullptr) return;

        // Base config
        bios_path     = ini->getString ("Emulation", "bios");
        multiplier    = ini->getInteger("Emulation", "fastforward");
        darken_screen = ini->getInteger("Video",     "darken");
        frameskip     = ini->getInteger("Video",     "frameskip");

        // Video configuration
        video.scale   = ini->getInteger("Video", "scale");
        video.keep_ar = ini->getInteger("Video", "aspectratio");
    
        // Audio quality
        audio.sample_rate = ini->getInteger("Audio", "sample_rate");
        audio.buffer_size = ini->getInteger("Audio", "buffer_size");

        // Audio disable/enable
        audio.mute.master = ini->getInteger("Audio", "mute");
        audio.mute.psg[0] = ini->getInteger("Audio", "mute_psg0");
        audio.mute.psg[1] = ini->getInteger("Audio", "mute_psg1");
        audio.mute.psg[2] = ini->getInteger("Audio", "mute_psg2");
        audio.mute.psg[3] = ini->getInteger("Audio", "mute_psg3");
        audio.mute.dma[0] = ini->getInteger("Audio", "mute_dma0");
        audio.mute.dma[1] = ini->getInteger("Audio", "mute_dma1");
    }
private:
    Util::INI* ini { nullptr };

};
