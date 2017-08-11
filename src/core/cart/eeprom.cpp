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

#include <iostream>

namespace GameBoyAdvance {
    constexpr int EEPROM::s_addr_bits[2];

    EEPROM::EEPROM(std::string save_path, EEPROMSize size) : size(size) {
        int bufferSize = (1 << s_addr_bits[size]) * 8;

        memory = new u8[bufferSize];
        currentAddress = 0;
        resetSerialBuffer();

        state = State::AcceptCommand;
    }

    EEPROM::~EEPROM() {
    }

    void EEPROM::reset() {
    }

    void EEPROM::resetSerialBuffer() {
        serialBuffer = 0;
        receivedBits = 0;
    }

    auto EEPROM::read8(u32 address) -> u8 {
        std::cout << "read! " << std::hex << address << std::dec << std::endl;
        return 0xFF;
    }

    void EEPROM::write8(u32 address, u8 value) {
        value &= 1;

        //std::cout << std::hex << address << ": " << (int)value << std::dec << std::endl;

        serialBuffer = (serialBuffer << 1) | value;
        receivedBits++;

        //std::cout << std::hex << serialBuffer << std::endl;

        switch (state) {
            case State::AcceptCommand:
                if (receivedBits == 2) {
                    // TODO: log invalid commands
                    switch (serialBuffer) {
                        case 2: state = State::GetWriteAddress; break;
                        case 3: state = State::GetReadAddress;  break;
                    }
                    resetSerialBuffer();
                }
                break;

            case State::GetReadAddress:
            case State::GetWriteAddress:
                if (receivedBits == s_addr_bits[size]) {
                    currentAddress = serialBuffer * 8;
                    std::cout << std::hex << "addr: " << currentAddress << std::endl;

                    // TODO: might need to erase all 8 bytes on write request
                    switch (state) { // meh!
                        case State::GetReadAddress:  state = State::PreReadData; break;
                        case State::GetWriteAddress: state = State::WriteData;   break;
                    }
                    resetSerialBuffer();
                }
                break;
        }
    }
}
