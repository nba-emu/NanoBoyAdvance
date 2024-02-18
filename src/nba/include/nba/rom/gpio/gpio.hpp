/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cassert>
#include <nba/rom/gpio/device.hpp>
#include <nba/integer.hpp>
#include <nba/save_state.hpp>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace nba {

struct GPIO {
  GPIO() {
    Reset();
  }

  void Reset();
  void Attach(std::shared_ptr<GPIODevice> device);

  template<typename T>
  auto Get() -> T* {
    auto match = device_map.find(std::type_index{typeid(T)});

    if(match != device_map.end()) {
      return (T*)match->second;
    }
    return nullptr;
  }

  bool IsReadable() const {
    return allow_reads;
  }

  auto Read (u32 address) -> u8;
  void Write(u32 address, u8 value);

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

private:
  enum class Register {
    Data = 0xC4,
    Direction = 0xC6,
    Control = 0xC8
  };

  bool allow_reads;
  u8 rd_mask;
  u8 wr_mask;
  u8 port_data;

  std::vector<std::shared_ptr<GPIODevice>> devices;

  std::unordered_map<std::type_index, GPIODevice*> device_map;
};

} // namespace nba
