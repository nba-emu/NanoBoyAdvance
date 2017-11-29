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
        bufferSize = (1 << s_addr_bits[size]) * 8;
        memory = new u8[bufferSize];

        reset();

        Logger::log<LOG_DEBUG>("EEPROM: buffer size is {0} (bytes).", bufferSize);
    }

    EEPROM::~EEPROM() {

        delete memory;
    }

    void EEPROM::reset() {
        std::memset(memory, 0, bufferSize);
        currentAddress = 0;
        resetSerialBuffer();

        state = EEPROM_ACCEPT_COMMAND;
    }

    void EEPROM::resetSerialBuffer() {
        serialBuffer    = 0;
        transmittedBits = 0;
    }

    auto EEPROM::read8(u32 address) -> u8 {
        if (state & EEPROM_READING) {
            if (state & EEPROM_DUMMY_NIBBLE) {
                // Four bits that simply are ignored but will be send.
                if (++transmittedBits == 4) {
                    state &= ~EEPROM_DUMMY_NIBBLE;
                    resetSerialBuffer();
                }
                return 0;
            }
            else {
                int bit   = transmittedBits % 8;
                int index = transmittedBits / 8;

                if (++transmittedBits == 64) {
                    Logger::log<LOG_DEBUG>("EEPROM: completed read. accepting new commands.");

                    state = EEPROM_ACCEPT_COMMAND;
                    resetSerialBuffer();
                }

                return (memory[currentAddress + index] >> (7 - bit)) & 1;
            }
        }

        return 0;
    }

    void EEPROM::write8(u32 address, u8 value) {
        if (state & EEPROM_READING) {
            return;
        }

        value &= 1;

        serialBuffer = (serialBuffer << 1) | value;
        transmittedBits++;

        if (state == EEPROM_ACCEPT_COMMAND && transmittedBits == 2) {
            switch (serialBuffer) {
                case 2: {
                    Logger::log<LOG_DEBUG>("EEPROM: receiving write request...");
                    state = EEPROM_WRITE_MODE  |
                            EEPROM_GET_ADDRESS |
                            EEPROM_WRITING     |
                            EEPROM_EAT_DUMMY;
                    break;
                }
                case 3: {
                    Logger::log<LOG_DEBUG>("EEPROM: receiving read request...");
                    state = EEPROM_READ_MODE   |
                            EEPROM_GET_ADDRESS |
                            EEPROM_EAT_DUMMY;
                    break;
                }
            }
            resetSerialBuffer();
        }
        else if (state & EEPROM_GET_ADDRESS) {
            if (transmittedBits == s_addr_bits[size]) {
                currentAddress = serialBuffer * 8;

                if (state & EEPROM_WRITE_MODE) {
                    // Clean memory 'cell'.
                    for (int i = 0; i < 8; i++) {
                        memory[currentAddress + i] = 0;
                    }
                }

                Logger::log<LOG_DEBUG>("EEPROM: requested address = 0x{0:X}", currentAddress);

                state &= ~EEPROM_GET_ADDRESS;
                resetSerialBuffer();
            }
        }
        else if (state & EEPROM_WRITING) {
            // - 1 is quick hack: in this case transmittedBits counts from 1-64 but we need 0-63...
            int bit   = (transmittedBits - 1) % 8;
            int index = (transmittedBits - 1) / 8;

            memory[currentAddress + index] &= ~(1 << (7 - bit));
            memory[currentAddress + index] |= value << (7 - bit);

            if (transmittedBits == 64) {
                Logger::log<LOG_DEBUG>("EEPROM: burned 64 bits of data.");

                state &= ~EEPROM_WRITING;
                resetSerialBuffer();
            }
        }
        else if (state & EEPROM_EAT_DUMMY) {
            if (serialBuffer != 0) {
                Logger::log<LOG_WARN>("EEPROM: received non-zero dummy bit.");
            }
            state &= ~EEPROM_EAT_DUMMY;

            if (state & EEPROM_READ_MODE) {
                Logger::log<LOG_DEBUG>("EEPROM: got dummy, read enabled.");

                state |= EEPROM_READING | EEPROM_DUMMY_NIBBLE;
            }
            else if (state & EEPROM_WRITE_MODE) {
                Logger::log<LOG_DEBUG>("EEPROM: write completed, accepting new commands.");

                state = EEPROM_ACCEPT_COMMAND;
            }

            resetSerialBuffer();
        }
    }
}
