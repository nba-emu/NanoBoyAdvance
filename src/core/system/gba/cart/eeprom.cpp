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

#include <cstring>
#include <iostream>

namespace Core {
    constexpr int EEPROM::s_addr_bits[2];

    EEPROM::EEPROM(std::string save_path, EEPROMSize size) : size(size) {
        int bufferSize = (1 << s_addr_bits[size]) * 8;

        std::cout << bufferSize << std::endl;

        memory = new u8[bufferSize];
        std::memset(memory, 0, bufferSize);
        currentAddress = 0;
        resetSerialBuffer();

        state = State::AcceptCommand;
    }

    EEPROM::~EEPROM() {
    }

    void EEPROM::reset() {
    }

    void EEPROM::resetSerialBuffer() {
        serialBuffer    = 0;
        transmittedBits = 0;
    }

    auto EEPROM::read8(u32 address) -> u8 {
        //std::string tmp;
        //std::cout << "Reading one bit!" << std::endl;
        //std::cin  >> tmp;

        if (state & State::EnableRead) {
            if (state & State::DummyNibble) {
                std::cout << "Read nibble bit " << std::dec << transmittedBits << std::endl;

                if (++transmittedBits == 4) {
                    state &= ~State::DummyNibble;
                    resetSerialBuffer();
                }
                return 0;
            }
            else {
                int bit   = transmittedBits % 8;
                int index = transmittedBits / 8;

                std::cout << "Read data bit " << std::dec << transmittedBits << std::endl;

                if (++transmittedBits == 64) {
                    state = State::AcceptCommand;
                    resetSerialBuffer();
                }

                return (memory[currentAddress + index] >> (7 - bit)) & 1;
            }
        }

        return 0;

        /*std::string x;
        std::cout << transmittedBits << std::endl;
        std::cin >> x;

        switch (state) {
            case State::PreReadData:
                // TODO: find out what the actual hardware returns for these 4 bits
                if (++transmittedBits == 4) {
                    state = State::ReadData;
                    resetSerialBuffer();
                }
                std::cout << "returning dummy bit..." << std::endl;
                return 0;

            case State::ReadData:
                if (++transmittedBits == 64) {
                    state = State::AcceptCommand;
                    resetSerialBuffer();
                    std::cout << "finished read!" << std::endl;
                }
                std::cout << "returning bit: " << std::dec << transmittedBits << std::endl;
                return 1;
                //return memory[currentAddress + (transmittedBits >> 3)];
        }*/

        return 0;
    }

    void EEPROM::write8(u32 address, u8 value) {
        if (state & State::EnableRead) {
            return;
        }

        value &= 1;

        //std::cout << std::hex << address << ": " << (int)value << std::dec << std::endl;

        serialBuffer = (serialBuffer << 1) | value;
        transmittedBits++;

        std::cout << "serialBuffer: " << std::hex << serialBuffer << std::endl;

        if (state == State::AcceptCommand && transmittedBits == 2) {
            switch (serialBuffer) {
                case 2: {
                    std::cout << "Begin Write command" << std::endl;
                    state = State::WriteMode  |
                            State::GetAddress |
                            State::GetWriteData |
                            State::EatDummy;
                    break;
                }
                case 3: {
                    std::cout << "Begin Read command" << std::endl;
                    state = State::ReadMode   |
                            State::GetAddress |
                            State::EatDummy;
                    break;
                }
            }
            resetSerialBuffer();
        }
        else if (state & State::GetAddress) {
            if (transmittedBits == s_addr_bits[size]) {
                currentAddress = serialBuffer * 8;

                if (state & State::WriteMode) {
                    std::cout << "Clean 8 bytes" << std::endl;
                    for (int i = 0; i < 8; i++) {
                        memory[currentAddress + i] = 0;
                    }
                }

                std::cout << "Received address: 0x" << std::hex << currentAddress << std::endl;
                state &= ~State::GetAddress;
                resetSerialBuffer();
            }
        }
        else if (state & State::GetWriteData) {
            // - 1 is quick hack: in this case transmittedBits counts from 1-64 but we need 0-63...
            int bit   = (transmittedBits - 1) % 8;
            int index = (transmittedBits - 1) / 8;

            std::cout << "Burning bit: " << std::to_string(value) << std::endl;

            memory[currentAddress + index] &= ~(1 << (7 - bit));
            memory[currentAddress + index] |= value << (7 - bit);

            if (transmittedBits == 64) {
                state &= ~State::GetWriteData;
                resetSerialBuffer();
            }
        }
        else if (state & State::EatDummy) {
            std::cout << "Ate dummy bit" << std::endl;
            if (serialBuffer != 0) {
                std::cout << "Warn: dummy but serial buffer is non-zero!" << std::endl;
            }
            state &= ~State::EatDummy;

            if (state & State::ReadMode) {
                state |= State::EnableRead | State::DummyNibble;
            }
            else if (state & State::WriteMode) {
                state = State::AcceptCommand;
            }

            resetSerialBuffer();
        }
    }
}
