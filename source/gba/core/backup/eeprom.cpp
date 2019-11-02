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

#include <cstring>
#include <exception>
#include <experimental/filesystem>
#include <fstream>

#include "eeprom.hpp"

using namespace GameBoyAdvance;

namespace fs = std::experimental::filesystem;

static constexpr int g_addr_bits[2] = { 6, 14 };
static constexpr int g_save_size[2] = { 512, 8192 };

EEPROM::EEPROM(std::string const& save_path, Size size_hint)
  : save_path(save_path)
  , size(size_hint)
{
  int bytes = g_save_size[size_hint];
  
  /* TODO: get rid of the ugly redundancy. */
  if (fs::exists(save_path)) {
    auto save_size = fs::file_size(save_path);
    
    if (save_size == g_save_size[0] || save_size == g_save_size[1]) {
      size = (save_size == g_save_size[0]) ? SIZE_4K : SIZE_64K;

      std::ifstream stream { save_path, std::ios::binary };

      /* TODO: find a proper way to handle this. */
      if (!stream.good())
        throw std::runtime_error("Cannot read save file.");

      memory = new std::uint8_t[save_size];
      stream.read((char*)memory, save_size);
      stream.close();
    } else {
      memory = new std::uint8_t[bytes];
      std::memset(memory, 0xFF, bytes);
    }
  } else {
    memory = new std::uint8_t[bytes];
    std::memset(memory, 0xFF, bytes);
  }
  
  Reset();
}

void EEPROM::Reset() {
  state = STATE_ACCEPT_COMMAND;
  address = 0;
  ResetSerialBuffer();
}

void EEPROM::ResetSerialBuffer() {
  serial_buffer = 0;
  transmitted_bits = 0;
}

#include <cstdio>

auto EEPROM::Read(std::uint32_t address) -> std::uint8_t {
  if (state & STATE_READING) {
    if (state & STATE_DUMMY_NIBBLE) {
      // Four bits that simply are ignored but will be send.
      if (++transmitted_bits == 4) {
        state &= ~STATE_DUMMY_NIBBLE;
        ResetSerialBuffer();
      }
      return 0;
    } else { /* TODO: else is kind of unnecessary. */
      int bit   = transmitted_bits % 8;
      int index = transmitted_bits / 8;

      if (++transmitted_bits == 64) {
        //Logger::log<LOG_DEBUG>("EEPROM: completed read. accepting new commands.");

        state = STATE_ACCEPT_COMMAND;
        ResetSerialBuffer();
      }

      std::printf("EEPROM::read: address=0x%08x index=%d\n", this->address, index);
      
      return (memory[this->address + index] >> (7 - bit)) & 1;
    }
  }

  return 0;
}

void EEPROM::Write(std::uint32_t address, std::uint8_t value) {
  if (state & STATE_READING) return;

  value &= 1;

  serial_buffer = (serial_buffer << 1) | value;
  transmitted_bits++;

  if (state == STATE_ACCEPT_COMMAND && transmitted_bits == 2) {
    switch (serial_buffer) {
      case 2: {
        //Logger::log<LOG_DEBUG>("EEPROM: receiving write request...");
        state = STATE_WRITE_MODE  |
                STATE_GET_ADDRESS |
                STATE_WRITING     |
                STATE_EAT_DUMMY;
        break;
      }
      case 3: {
        //Logger::log<LOG_DEBUG>("EEPROM: receiving read request...");
        state = STATE_READ_MODE   |
                STATE_GET_ADDRESS |
                STATE_EAT_DUMMY;
        break;
      }
    }
    ResetSerialBuffer();
  } else if (state & STATE_GET_ADDRESS) {
    if (transmitted_bits == g_addr_bits[size]) {
      this->address = serial_buffer * 8;

      if (state & STATE_WRITE_MODE) {
        // Clean memory 'cell'.
        for (int i = 0; i < 8; i++) {
          memory[this->address + i] = 0;
        }
      }

      //Logger::log<LOG_DEBUG>("EEPROM: requested address = 0x{0:X}", this->address);

      state &= ~STATE_GET_ADDRESS;
      ResetSerialBuffer();
    }
  } else if (state & STATE_WRITING) {
    // - 1 is quick hack: in this case transmitted_bits counts from 1-64 but we need 0-63...
    int bit   = (transmitted_bits - 1) % 8;
    int index = (transmitted_bits - 1) / 8;

    memory[this->address + index] &= ~(1 << (7 - bit));
    memory[this->address + index] |= value << (7 - bit);

    if (transmitted_bits == 64) {
      //Logger::log<LOG_DEBUG>("EEPROM: burned 64 bits of data.");

      state &= ~STATE_WRITING;
      ResetSerialBuffer();
    }
  } else if (state & STATE_EAT_DUMMY) {
    if (serial_buffer != 0) {
      //Logger::log<LOG_WARN>("EEPROM: received non-zero dummy bit.");
    }
    
    state &= ~STATE_EAT_DUMMY;

    if (state & STATE_READ_MODE) {
      //Logger::log<LOG_DEBUG>("EEPROM: got dummy, read enabled.");

      state |= STATE_READING | STATE_DUMMY_NIBBLE;
    } else if (state & STATE_WRITE_MODE) {
      //Logger::log<LOG_DEBUG>("EEPROM: write completed, accepting new commands.");

      state = STATE_ACCEPT_COMMAND;
    }

    ResetSerialBuffer();
  }
}