/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

namespace NanoboyAdvance::GBA {

struct EventDevice {
    int wait_cycles = 0;
    
    virtual bool Tick() = 0;
};

}