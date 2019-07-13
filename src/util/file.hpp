/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <fstream>
#include <string>

namespace File {

/*
auto Exists(std::string const& path) -> bool {
    std::ifstream stream { path };
    bool result = stream.is_open();
    stream.close();
    return result;
}

auto GetSize(std::string const& path) -> size_t {
    std::ifstream stream { path, std::ios::in | std::ios::binary | std::ios::ate };
    size_t size = 0;
    
    if (stream.is_open()) {
        stream.seekg(0, std::ios::end);
        size = stream.tellg();
        stream.close();
    }
    
    return size;
}
*/

}