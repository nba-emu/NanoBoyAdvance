/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>
#include <exception>
#include <nba/rom/backup/eeprom.hpp>

#include <nba/log.hpp>

using namespace nba::core;

namespace nba {

static constexpr int g_addr_bits[2] = { 6, 14 };
static constexpr int g_save_size[2] = { 512, 8192 };

EEPROM::EEPROM(fs::path const& save_path, Size size_hint, core::Scheduler& scheduler)
    : size(size_hint)
    , save_path(save_path)
    , scheduler(scheduler) {
  scheduler.Register(Scheduler::EventClass::EEPROM_ready, this, &EEPROM::OnReadyAfterWrite);
  
  Reset();
}

void EEPROM::Reset() {
  state = STATE_ACCEPT_COMMAND;
  address = 0;
  ResetSerialBuffer();

  if(size == DETECT) {
    size = SIZE_64K; // default to 8 KiB size
    detect_size = true;
  } else {
    detect_size = false;
  }

  int bytes = g_save_size[size];
  
  file = BackupFile::OpenOrCreate(save_path, {512, 8192}, bytes);

  if(bytes == g_save_size[0]) {
    size = SIZE_4K;
  } else {
    size = SIZE_64K;
  }
}

void EEPROM::ResetSerialBuffer() {
  serial_buffer = 0;
  transmitted_bits = 0;
}

auto EEPROM::Read(u32 address) -> u8 {
  if(state & STATE_READING) {
    if(state & STATE_DUMMY_NIBBLE) {
      // Four bits that simply are ignored but will be send.
      if(++transmitted_bits == 4) {
        state &= ~STATE_DUMMY_NIBBLE;
        ResetSerialBuffer();
      }
      return 0;
    }
    
    int bit   = transmitted_bits % 8;
    int index = transmitted_bits / 8;

    if(++transmitted_bits == 64) {
      state = STATE_ACCEPT_COMMAND;
      ResetSerialBuffer();
    }
      
    return (file->Read(this->address + index) >> (7 - bit)) & 1;
  }

  return (state & STATE_BUSY) ? 0 : 1;
}

void EEPROM::Write(u32 address, u8 value) {
  if(state & (STATE_READING | STATE_BUSY)) return;

  value &= 1;

  serial_buffer = (serial_buffer << 1) | value;
  transmitted_bits++;

  if(state == STATE_ACCEPT_COMMAND && transmitted_bits == 2) {
    switch(serial_buffer) {
      case 2: {
        state = STATE_WRITE_MODE  |
                STATE_GET_ADDRESS |
                STATE_WRITING     |
                STATE_EAT_DUMMY;
        break;
      }
      case 3: {
        state = STATE_READ_MODE   |
                STATE_GET_ADDRESS |
                STATE_EAT_DUMMY;
        break;
      }
    }
    ResetSerialBuffer();
  } else if(state & STATE_GET_ADDRESS) {
    if(transmitted_bits == g_addr_bits[size]) {
      this->address  = serial_buffer * 8;
      this->address &= 0x1FFF;

      if(state & STATE_WRITE_MODE) {
        file->MemorySet(this->address, 8, 0);
      }

      state &= ~STATE_GET_ADDRESS;
      ResetSerialBuffer();
    }
  } else if(state & STATE_WRITING) {
    int bit   = (transmitted_bits - 1) % 8;
    int index = (transmitted_bits - 1) / 8;

    auto tmp = file->Read(this->address + index);
    tmp |= value << (7 - bit);
    file->Write(this->address + index, tmp);
    
    // @todo: should the EEPROM be reprogrammed if the dummy bit is never read?
    if(transmitted_bits == 64) {
      state &= ~STATE_WRITING;
      ResetSerialBuffer();
    }
  } else if(state & STATE_EAT_DUMMY) {
    state &= ~STATE_EAT_DUMMY;

    if(state & STATE_READ_MODE) {
      state |= STATE_READING | STATE_DUMMY_NIBBLE;
    } else if(state & STATE_WRITE_MODE) {
      /**
       * It takes ~6 ms for the EEPROM chip to actually write the data.
       * Presumably a new command cannot be submitted during those ~6 ms.
       * The exact timing varies based on environmental temperature and likely from cartridge to cartridge. 
       */ 
      state = STATE_BUSY;
      scheduler.Add(101400, Scheduler::EventClass::EEPROM_ready);
    }

    ResetSerialBuffer();
  }
}

void EEPROM::SetSizeHint(Size size) {
  if(detect_size) {
    int bytes = g_save_size[size];

    this->size = size;
    detect_size = false;

    if(file->Size() != bytes) {
      file = BackupFile::OpenOrCreate(save_path, {(size_t)bytes}, bytes);
    }
  }
}

void EEPROM::OnReadyAfterWrite() {
  state = STATE_ACCEPT_COMMAND;
}

} // namespace nba
