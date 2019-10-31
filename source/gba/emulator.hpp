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

#pragma once

#include <memory>
#include <string>

#include "core/cpu.hpp"

namespace GameBoyAdvance {
  
class Emulator {
public:
  enum class StatusCode {
    BiosNotFound,
    GameNotFound,
    BiosWrongSize,
    GameWrongSize,
    Ok
  };
  
  Emulator(std::shared_ptr<Config> config);

  void Reset();
  auto LoadGame(std::string const& path) -> StatusCode;
  void Frame();
  
private:
  auto LoadBIOS() -> StatusCode; 
  
  CPU cpu;
  bool bios_loaded = false;
  std::shared_ptr<Config> config;
};

}