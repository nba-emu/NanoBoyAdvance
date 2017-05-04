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

inline u32 read_byte(u32 address, int flags) {
    u32 value = bus_read_byte(address);
    
    if ((flags & MEM_SIGNED) && (value & 0x80)) {
        return value | 0xFFFFFF00;
    }
    
    return value;
}

inline u32 read_hword(u32 address, int flags) {
    
    u32 value;
    
    if (flags & MEM_ROTATE) {
        if (address & 1) {
            value = bus_read_hword(address & ~1);
            return (value >> 8) | (value << 24);
        }
        return bus_read_hword(address);
    }
    
    if (flags & MEM_SIGNED) {
        if (address & 1) {
            value = bus_read_byte(address);
            if (value & 0x80) {
                return value | 0xFFFFFF00;
            }
            return value;
        } else {
            value = bus_read_hword(address);
            if (value & 0x8000) {
                return value | 0xFFFF0000;
            }
            return value;
        }
    }
    
    return bus_read_hword(address & ~1);
}

inline u32 read_word(u32 address, int flags) {
    u32 value = bus_read_word(address & ~3);
    
    if (flags & MEM_ROTATE) {
        int amount = (address & 3) << 3;
        return (value >> amount) | (value << (32 - amount));
    }
    
    return value;
}

inline void write_byte(u32 address, u8 value, int flags) {
    bus_write_byte(address, value);
}

inline void write_hword(u32 address, u16 value, int flags) {
    bus_write_hword(address & ~1, value);
}

inline void write_word(u32 address, u32 value, int flags) {
    bus_write_word(address & ~3, value);
}

inline void refill_pipeline() {
    if (ctx.cpsr & MASK_THUMB) {
        ctx.pipe.opcode[0] = bus_read_hword(ctx.r15);
        ctx.pipe.opcode[1] = bus_read_hword(ctx.r15 + 2);
        ctx.r15 += 4;
    } else {
        ctx.pipe.opcode[0] = bus_read_word(ctx.r15);
        ctx.pipe.opcode[1] = bus_read_word(ctx.r15 + 4);
        ctx.r15 += 8;
    }
    
    ctx.pipe.index = 0;
    ctx.pipe.do_flush = false;
}

