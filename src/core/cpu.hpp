/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "arm/arm.hpp"
#include "arm/interface.hpp"

namespace NanoboyAdvance {
namespace GBA {

class CPU : ARM::Interface {

public:
    void Init() {
        cpu.Reset();
        cpu.SetInterface(this);
    }
    
    virtual std::uint8_t  ReadByte(std::uint32_t address, ARM::AccessType type) = 0;
    virtual std::uint16_t ReadHalf(std::uint32_t address, ARM::AccessType type) = 0;
    virtual std::uint32_t ReadWord(std::uint32_t address, ARM::AccessType type) = 0;

    virtual void WriteByte(std::uint32_t address, std::uint8_t  value, ARM::AccessType type) = 0;
    virtual void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) = 0;
    virtual void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) = 0;

private:
    ARM::ARM7 cpu;

};

} // namespace GBA
} // namespace NanoboyAdvance
