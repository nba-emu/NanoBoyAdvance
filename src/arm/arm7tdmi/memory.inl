/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
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

#ifndef ARM_INCLUDE_GUARD
#error "This file cannot be included regularely!"
#endif

std::uint32_t ReadByte(std::uint32_t address, AccessType type) {
  return interface->ReadByte(address, type);
}

std::uint32_t ReadHalf(std::uint32_t address, AccessType type) {
  return interface->ReadHalf(address & ~1, type);
}

std::uint32_t ReadWord(std::uint32_t address, AccessType type) {
  return interface->ReadWord(address & ~3, type);
}

std::uint32_t ReadByteSigned(std::uint32_t address, AccessType type) {
  std::uint32_t value = interface->ReadByte(address, type);

  if (value & 0x80) {
    value |= 0xFFFFFF00;
  }

  return value;
}

std::uint32_t ReadHalfRotate(std::uint32_t address, AccessType type) {
  auto value = interface->ReadHalf(address & ~1, type);

  if (address & 1) {
    value = (value >> 8) | (value << 24);
  }

  return value;
}

std::uint32_t ReadHalfSigned(std::uint32_t address, AccessType type) {
  std::uint32_t value;

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

std::uint32_t ReadWordRotate(std::uint32_t address, AccessType type) {
  auto value = interface->ReadWord(address & ~3, type);
  auto shift = (address & 3) * 8;

  return (value >> shift) | (value << (32 - shift));
}

void WriteByte(std::uint32_t address, std::uint8_t  value, AccessType type) {
  interface->WriteByte(address, value, type);
}

void WriteHalf(std::uint32_t address, std::uint16_t value, AccessType type) {
  interface->WriteHalf(address & ~1, value, type);
}

void WriteWord(std::uint32_t address, std::uint32_t value, AccessType type) {
  interface->WriteWord(address & ~3, value, type);
}
