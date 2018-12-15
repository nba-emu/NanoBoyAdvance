/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>
#include "breakpoint.hpp"

namespace ARM {

struct Debugger {
    virtual void OnHit(Breakpoint* breakpoint) = 0;
    virtual auto Get(Breakpoint::Type type) -> std::vector<Breakpoint*>& = 0;
};

} // namespace ARM
