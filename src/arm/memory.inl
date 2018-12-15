/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef DEBUGGER

inline void ARM7::DebuggerCheckRead(std::uint32_t address) {
    if (debugger == nullptr)
        return;
    for (auto breakpoint : debugger->Get(Breakpoint::Type::Read)) {
        if (address == breakpoint->address) {
            breakpoint->hitTimes++;
            debugger->OnHit(breakpoint);
            break;
        }
    }
}

inline void ARM7::DebuggerCheckWrite(std::uint32_t address) {
    if (debugger == nullptr)
        return;
    for (auto breakpoint : debugger->Get(Breakpoint::Type::Write)) {
        if (address == breakpoint->address) {
            breakpoint->hitTimes++;
            debugger->OnHit(breakpoint);
            break;
        }
    }
}

#endif

inline std::uint32_t ARM7::ReadByte(std::uint32_t address, AccessType type) {
#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    return interface->ReadByte(address, type);
}

inline std::uint32_t ARM7::ReadHalf(std::uint32_t address, AccessType type) {
#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    return interface->ReadHalf(address & ~1, type);
}

inline std::uint32_t ARM7::ReadWord(std::uint32_t address, AccessType type) {
#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    return interface->ReadWord(address & ~3, type);
}

inline std::uint32_t ARM7::ReadByteSigned(std::uint32_t address, AccessType type) {
    std::uint32_t value = interface->ReadByte(address, type);

#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    if (value & 0x80) {
        value |= 0xFFFFFF00;
    }

    return value;
}

inline std::uint32_t ARM7::ReadHalfRotate(std::uint32_t address, AccessType type) {
    auto value = interface->ReadHalf(address & ~1, type);

#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    if (address & 1) {
        value = (value >> 8) | (value << 24);
    }

    return value;
}

inline std::uint32_t ARM7::ReadHalfSigned(std::uint32_t address, AccessType type) {
    std::uint32_t value;

#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    if (address & 1) {
        value = interface->ReadByte(address, type);
        if (value & 0x80) {
            value |= 0xFFFFFF00;
        }
    } else {
        value = interface->ReadHalf(address, type);
        if (value & 0x8000) {
            value |= 0xFFFF0000;
        }
    }

    return value;
}

inline std::uint32_t ARM7::ReadWordRotate(std::uint32_t address, AccessType type) {
    auto value = interface->ReadWord(address & ~3, type);
    auto shift = (address & 3) * 8;

#ifdef DEBUGGER
    DebuggerCheckRead(address);
#endif
    return (value >> shift) | (value << (32 - shift));
}

inline void ARM7::WriteByte(std::uint32_t address, std::uint8_t  value, AccessType type) {
    interface->WriteByte(address, value, type);
#ifdef DEBUGGER
    DebuggerCheckWrite(address);
#endif
}

inline void ARM7::WriteHalf(std::uint32_t address, std::uint16_t value, AccessType type) {
    interface->WriteHalf(address & ~1, value, type);
#ifdef DEBUGGER
    DebuggerCheckWrite(address);
#endif
}

inline void ARM7::WriteWord(std::uint32_t address, std::uint32_t value, AccessType type) {
    interface->WriteWord(address & ~3, value, type);
#ifdef DEBUGGER
    DebuggerCheckWrite(address);
#endif
}
