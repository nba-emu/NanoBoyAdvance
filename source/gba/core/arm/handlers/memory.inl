/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

std::uint32_t ReadByte(std::uint32_t address, Access access) {
  return interface->ReadByte(address, access);
}

std::uint32_t ReadHalf(std::uint32_t address, Access access) {
  return interface->ReadHalf(address & ~1, access);
}

std::uint32_t ReadWord(std::uint32_t address, Access access) {
  return interface->ReadWord(address & ~3, access);
}

std::uint32_t ReadByteSigned(std::uint32_t address, Access access) {
  std::uint32_t value = interface->ReadByte(address, access);

  if (value & 0x80) {
    value |= 0xFFFFFF00;
  }

  return value;
}

std::uint32_t ReadHalfRotate(std::uint32_t address, Access access) {
  std::uint32_t value = interface->ReadHalf(address & ~1, access);

  if (address & 1) {
    value = (value >> 8) | (value << 24);
  }

  return value;
}

std::uint32_t ReadHalfSigned(std::uint32_t address, Access access) {
  std::uint32_t value;

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

std::uint32_t ReadWordRotate(std::uint32_t address, Access access) {
  auto value = interface->ReadWord(address & ~3, access);
  auto shift = (address & 3) * 8;

  return (value >> shift) | (value << (32 - shift));
}

void WriteByte(std::uint32_t address, std::uint8_t  value, Access access) {
  interface->WriteByte(address, value, access);
}

void WriteHalf(std::uint32_t address, std::uint16_t value, Access access) {
  interface->WriteHalf(address & ~1, value, access);
}

void WriteWord(std::uint32_t address, std::uint32_t value, Access access) {
  interface->WriteWord(address & ~3, value, access);
}
