/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

u32 ReadByte(u32 address, Access access) {
  return interface->ReadByte(address, access);
}

u32 ReadHalf(u32 address, Access access) {
  return interface->ReadHalf(address, access);
}

u32 ReadWord(u32 address, Access access) {
  return interface->ReadWord(address, access);
}

u32 ReadByteSigned(u32 address, Access access) {
  u32 value = interface->ReadByte(address, access);

  if (value & 0x80) {
    value |= 0xFFFFFF00;
  }

  return value;
}

u32 ReadHalfRotate(u32 address, Access access) {
  u32 value = interface->ReadHalf(address, access);

  if (address & 1) {
    value = (value >> 8) | (value << 24);
  }

  return value;
}

u32 ReadHalfSigned(u32 address, Access access) {
  u32 value;

  if (address & 1) {
    value = interface->ReadByte(address, access);
    if (value & 0x80) {
      value |= 0xFFFFFF00;
    }
  } else {
    value = interface->ReadHalf(address, access);
    if (value & 0x8000) {
      value |= 0xFFFF0000;
    }
  }

  return value;
}

u32 ReadWordRotate(u32 address, Access access) {
  auto value = interface->ReadWord(address, access);
  auto shift = (address & 3) * 8;

  return (value >> shift) | (value << (32 - shift));
}

void WriteByte(u32 address, u8  value, Access access) {
  interface->WriteByte(address, value, access);
}

void WriteHalf(u32 address, u16 value, Access access) {
  interface->WriteHalf(address, value, access);
}

void WriteWord(u32 address, u32 value, Access access) {
  interface->WriteWord(address, value, access);
}
