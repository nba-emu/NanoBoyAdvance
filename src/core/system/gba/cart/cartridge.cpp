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

#include <map>
#include <cstring>
#include <stdexcept>
#include "cartridge.hpp"
#include "util/file.hpp"

#include "sram.hpp"
#include "flash.hpp"
#include "eeprom.hpp"

using namespace Util;

namespace Core {

    auto Cartridge::fromFile(std::string path, SaveType type) -> std::shared_ptr<Cartridge> {
        auto cart = new Cartridge();

        // Load game from drive
        if (!File::exists(path)) {
            //TODO: better exception system!
            throw std::runtime_error("file not found: " + path);
        }
        cart->data = File::read_data(path);
        cart->size = File::get_size (path);

        if (type == SAVE_DETECT) {
            cart->type = cart->detectType();
        } else {
            cart->type = type;
        }

        std::string save_path = path.substr(0, path.find_last_of(".")) + ".sav";

        switch (cart->type) {
            case SAVE_SRAM:     cart->backup = new SRAM (save_path)                  ; break;
            case SAVE_FLASH64:  cart->backup = new Flash(save_path, false)           ; break;
            case SAVE_FLASH128: cart->backup = new Flash(save_path, true )           ; break;
            case SAVE_EEPROM:   cart->backup = new EEPROM(save_path, EEPROM::SIZE_4K); break;
        }

        return std::shared_ptr<Cartridge>(cart);
    }

    // TODO: Have a list of game codes with the matching save types.
    auto Cartridge::detectType() -> SaveType {
        const std::map<std::string, SaveType> types {
            { "EEPROM_V",   SAVE_EEPROM   },
            { "SRAM_V",     SAVE_SRAM     },
            { "FLASH_V",    SAVE_FLASH64  },
            { "FLASH512_V", SAVE_FLASH64  },
            { "FLASH1M_V",  SAVE_FLASH128 }
        };

        for (int i = 0; i < size; i += 4) {
            for (auto& pair : types) {
                auto& str = pair.first;

                if (std::memcmp(&data[i], str.c_str(), str.size()) == 0) {
                    return pair.second;
                }
            }
        }

        return SAVE_DETECT; // TODO: introduce SAVE_NONE?
    }

}
