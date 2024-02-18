/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

u32 ReadByte(u32 address, int access) {
  return bus.ReadByte(address, access);
}

u32 ReadHalf(u32 address, int access) {
  return bus.ReadHalf(address, access);
}

u32 ReadWord(u32 address, int access) {
  return bus.ReadWord(address, access);
}

u32 ReadByteSigned(u32 address, int access) {
  u32 value = bus.ReadByte(address, access);

  if (value & 0x80) {
    value |= 0xFFFFFF00;
  }

  return value;
}

u32 ReadHalfRotate(u32 address, int access) {
  u32 value = bus.ReadHalf(address, access);

  if (address & 1) {
    value = (value >> 8) | (value << 24);
  }

  return value;
}

u32 ReadHalfSigned(u32 address, int access) {
  u32 value;

  if (address & 1) {
    value = bus.ReadByte(address, access);
    if (value & 0x80) {
      value |= 0xFFFFFF00;
    }
  } else {
    value = bus.ReadHalf(address, access);
    if (value & 0x8000) {
      value |= 0xFFFF0000;
    }
  }

  return value;
}

u32 ReadWordRotate(u32 address, int access) {
  auto value = bus.ReadWord(address, access);
  auto shift = (address & 3) * 8;

  return (value >> shift) | (value << (32 - shift));
}

void WriteByte(u32 address, u8  value, int access) {
  bus.WriteByte(address, value, access);
}

void WriteHalf(u32 address, u16 value, int access) {
  bus.WriteHalf(address, value, access);
}

void WriteWord(u32 address, u32 value, int access) {
  bus.WriteWord(address, value, access);
}
