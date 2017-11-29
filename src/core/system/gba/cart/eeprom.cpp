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

#include "eeprom.hpp"
#include "util/logger.hpp"

#include <cstring>

using namespace Util;

namespace Core {
    constexpr int EEPROM::s_addr_bits[2];

    EEPROM::EEPROM(std::string save_path, EEPROMSize size) : size(size) {
        memory_size = (1 << s_addr_bits[size]) * 8;
        memory = new u8[memory_size];

        reset();

        Logger::log<LOG_DEBUG>("EEPROM: buffer size is {0} (bytes).", memory_size);
    }

    EEPROM::~EEPROM() {

        delete memory;
    }

    void EEPROM::reset() {
        std::memset(memory, 0, memory_size);
        address = 0;
        resetSerialBuffer();

        state = STATE_ACCEPT_COMMAND;
    }

    void EEPROM::resetSerialBuffer() {
        serial_buffer    = 0;
        transmitted_bits = 0;
    }

    auto EEPROM::read8(u32 address) -> u8 {
        if (state & STATE_READING) {
            if (state & STATE_DUMMY_NIBBLE) {
                // Four bits that simply are ignored but will be send.
                if (++transmitted_bits == 4) {
                    state &= ~STATE_DUMMY_NIBBLE;
                    resetSerialBuffer();
                }
                return 0;
            }
            else {
                int bit   = transmitted_bits % 8;
                int index = transmitted_bits / 8;

                if (++transmitted_bits == 64) {
                    Logger::log<LOG_DEBUG>("EEPROM: completed read. accepting new commands.");

                    state = STATE_ACCEPT_COMMAND;
                    resetSerialBuffer();
                }

                return (memory[this->address + index] >> (7 - bit)) & 1;
            }
        }

        return 0;
    }

    void EEPROM::write8(u32 address, u8 value) {
        if (state & STATE_READING) {
            return;
        }

        value &= 1;

        serial_buffer = (serial_buffer << 1) | value;
        transmitted_bits++;

        if (state == STATE_ACCEPT_COMMAND && transmitted_bits == 2) {
            switch (serial_buffer) {
                case 2: {
                    Logger::log<LOG_DEBUG>("EEPROM: receiving write request...");
                    state = STATE_WRITE_MODE  |
                            STATE_GET_ADDRESS |
                            STATE_WRITING     |
                            STATE_EAT_DUMMY;
                    break;
                }
                case 3: {
                    Logger::log<LOG_DEBUG>("EEPROM: receiving read request...");
                    state = STATE_READ_MODE   |
                            STATE_GET_ADDRESS |
                            STATE_EAT_DUMMY;
                    break;
                }
            }
            resetSerialBuffer();
        }
        else if (state & STATE_GET_ADDRESS) {
            if (transmitted_bits == s_addr_bits[size]) {
                this->address = serial_buffer * 8;

                if (state & STATE_WRITE_MODE) {
                    // Clean memory 'cell'.
                    for (int i = 0; i < 8; i++) {
                        memory[this->address + i] = 0;
                    }
                }

                Logger::log<LOG_DEBUG>("EEPROM: requested address = 0x{0:X}", this->address);

                state &= ~STATE_GET_ADDRESS;
                resetSerialBuffer();
            }
        }
        else if (state & STATE_WRITING) {
            // - 1 is quick hack: in this case transmitted_bits counts from 1-64 but we need 0-63...
            int bit   = (transmitted_bits - 1) % 8;
            int index = (transmitted_bits - 1) / 8;

            memory[this->address + index] &= ~(1 << (7 - bit));
            memory[this->address + index] |= value << (7 - bit);

            if (transmitted_bits == 64) {
                Logger::log<LOG_DEBUG>("EEPROM: burned 64 bits of data.");

                state &= ~STATE_WRITING;
                resetSerialBuffer();
            }
        }
        else if (state & STATE_EAT_DUMMY) {
            if (serial_buffer != 0) {
                Logger::log<LOG_WARN>("EEPROM: received non-zero dummy bit.");
            }
            state &= ~STATE_EAT_DUMMY;

            if (state & STATE_READ_MODE) {
                Logger::log<LOG_DEBUG>("EEPROM: got dummy, read enabled.");

                state |= STATE_READING | STATE_DUMMY_NIBBLE;
            }
            else if (state & STATE_WRITE_MODE) {
                Logger::log<LOG_DEBUG>("EEPROM: write completed, accepting new commands.");

                state = STATE_ACCEPT_COMMAND;
            }

            resetSerialBuffer();
        }
    }
}
