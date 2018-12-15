/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdint>

namespace ARM {

struct Breakpoint {
    enum class Type {
        Code,
        Read,
        Write,
        SWI
    };
    
    Type type;
    int hitTimes;
    std::uint32_t address;

    Breakpoint(Type type, std::uint32_t address) : type(type),
                                                   hitTimes(0),
                                                   address(address) { }
};

} // namespace ARM
