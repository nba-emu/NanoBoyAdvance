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

inline auto ARM::read8(u32 address, int flags) -> u32 {
    u32 value = busRead8(address, flags);

    if ((flags & M_SIGNED) && (value & 0x80)) {
        return value | 0xFFFFFF00;
    }

    return value;
}

inline auto ARM::read16(u32 address, int flags) -> u32 {
    u32 value;

    if (flags & M_ROTATE) {
        if (address & 1) {
            value = busRead16(address & ~1, flags);
            return (value >> 8) | (value << 24);
        }
        return busRead16(address, flags);
    }

    if (flags & M_SIGNED) {
        if (address & 1) {
            value = busRead8(address, flags);
            if (value & 0x80) {
                return value | 0xFFFFFF00;
            }
            return value;
        } else {
            value = busRead16(address, flags);
            if (value & 0x8000) {
                return value | 0xFFFF0000;
            }
            return value;
        }
    }

    return busRead16(address & ~1, flags);
}

inline auto ARM::read32(u32 address, int flags) -> u32 {
    u32 value = busRead32(address & ~3, flags);

    if (flags & M_ROTATE) {
        int amount = (address & 3) << 3;
        return (value >> amount) | (value << (32 - amount));
    }

    return value;
}

inline void ARM::write8(u32 address, u8 value, int flags) {
    busWrite8(address, value, flags);
}

inline void ARM::write16(u32 address, u16 value, int flags) {
    busWrite16(address & ~1, value, flags);
}

inline void ARM::write32(u32 address, u32 value, int flags) {
    busWrite32(address & ~3, value, flags);
}
