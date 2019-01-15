/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance::GBA {

struct Config {
    struct Video {
        std::uint32_t* output = nullptr;
    } video;
};

}
