///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#ifndef __NBA_SDL_AUDIO_ADAPTER_H__
#define __NBA_SDL_AUDIO_ADAPTER_H__


#include "sdl_adapter.h"
#include "config/config.h"
#include "util/log.h"
#include <SDL2/SDL.h>


namespace GBA
{
    void SDL2AudioAdapter::Init(Audio* audio)
    {
        SDL_AudioSpec spec;
        Config config("config.sml");

        spec.freq = config.ReadInt("Audio::Quality", "SampleRate");
        spec.samples = config.ReadInt("Audio::Quality", "BufferSize");
        spec.format = AUDIO_U16;
        spec.channels = 2;
        spec.callback = AudioCallback;
        spec.userdata = audio;

        if (SDL_Init(SDL_INIT_AUDIO) < 0)
            LOG(LOG_ERROR, "SDL_Init(SDL_INIT_AUDIO) failed");

        if (SDL_OpenAudio(&spec, NULL) < 0)
            LOG(LOG_ERROR, "SDL_OpenAudio failed.");

        SDL_PauseAudio(0);
    }

    void SDL2AudioAdapter::Deinit()
    {
        SDL_CloseAudio();
    }

    void SDL2AudioAdapter::Pause()
    {
        SDL_PauseAudio(1);
    }

    void SDL2AudioAdapter::Resume()
    {
        SDL_PauseAudio(0);
    }
}


#endif
