/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace File {
    
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

auto ReadData(std::string const& path, std::uint8_t* data, size_t max_size) -> bool {
    std::ifstream stream { path, std::ios::in | std::ios::binary | std::ios::ate };
    size_t size;
    
    if (stream.is_open()) {
        stream.seekg(0, std::ios::end);
        size = stream.tellg();
        stream.seekg(0, std::ios::beg);
        if (size > max_size) {
            size = max_size;
        }
        stream.read((char*)data, size);
        stream.close();
        return true;
    }
    
    return false;
}

auto WriteData(std::string const& path, std::uint8_t* data, size_t size) -> bool {
    std::ofstream stream { path, std::ios::out | std::ios::binary };
    
    if (stream.is_open()) {
        stream.write((char*)data, size);
        stream.close();
        return true;
    }
    
    return false;
}

}