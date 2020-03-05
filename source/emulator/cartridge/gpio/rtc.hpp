/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "gpio.hpp"

namespace nba {

class RTC : public GPIO {
public:
  enum class Port {
    SCK,
    SIO,
    CS
  };

  enum class State {
    Waiting,
    Sending,
    Receiving,
    Complete
  };

  enum class Register {
    ForceReset = 0,
    DateTime = 2,
    ForceIRQ = 3,
    Control = 4,
    Time = 6,
    FreeRegister = 7
  };

  RTC(nba::core::Scheduler* scheduler, nba::core::InterruptController* irq_controller) : GPIO(scheduler, irq_controller) {
    Reset();
  }

  void Reset();

protected:
  auto ReadPort() -> std::uint8_t final;
  void WritePort(std::uint8_t value) final;

private:
  bool ReadSIO();
  void ReceiveCommandSIO();
  void ReceiveBufferSIO();
  void TransmitBufferSIO();
  void ReadRegister();
  void WriteRegister();

  int current_bit;
  int current_byte;

  Register reg;
  std::uint8_t data;
  std::uint8_t buffer[7];

  struct PortData {
    int sck;
    int sio;
    int cs;
  } port;

  State state;

  struct ControlRegister {
    bool unknown;
    bool per_minute_irq;
    bool mode_24h;
    bool poweroff;

    void Reset() {
      unknown = false;
      per_minute_irq = false;
      mode_24h = false;
      poweroff = false;
    }
  } control;

  struct DateTimeRegister {
    std::uint8_t year;
    std::uint8_t month;
    std::uint8_t day;
    std::uint8_t day_of_week;
    // NOTE: hour[7:] is AM/PM-flag, probably only in 12-hour mode?
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;

    void Reset() {
      year = 0;
      month = 1;
      day = 1;
      day_of_week = 0;
      hour = 0;
      minute = 0;
      second = 0;
    }
  } datetime;

  static constexpr int s_argument_count[8] = {
    0, // ForceReset
    0, // Unused?
    7, // DateTime
    0, // ForceIRQ
    1, // Control
    0, // Unused?
    3, // Time
    0  // FreeRegister
  };
};

} // namespace nba
