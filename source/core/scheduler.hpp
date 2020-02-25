/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <functional>
#include <limits>
#include <unordered_set>

namespace nba::core {

struct Event {
  int countdown = 0;
  std::function<void()> tick;
};

class Scheduler {
public:
  Scheduler() { Reset(); }

  void Reset() {
    events.clear();
    next_event = 0;
    cpu_cycles = 0;
    total_cpu_cycles = 0;
  }

  void AddCycles(int cycles) {
    cpu_cycles += cycles;
    total_cpu_cycles += cycles;
  }

  auto GetRemainingCycleCount() const -> int {
    return next_event - cpu_cycles;
  }

  auto TotalCycleCount() -> int& {
    return total_cpu_cycles;
  }
  
  void Add(Event& event) {
    events.insert(&event);
    /* If during emulation an event is added that should happen before
     * the next time we would synchronize otherwise, then we need to
     * determine a new, earlier synchronization point.
     */
    if (event.countdown < GetRemainingCycleCount()) {
      Schedule();
    }
  }

  void Remove(Event& event) {
    events.erase(&event);
  }
  
  void Schedule() {
    next_event = std::numeric_limits<int>::max();

    for (auto it = events.begin(); it != events.end();) {
      auto event = *it;
      ++it;
      event->countdown -= cpu_cycles;
      if (event->countdown <= 0) {
        event->tick();
      }
      if (event->countdown < next_event) {
        next_event = event->countdown;
      }
    }

    cpu_cycles = 0;
  }

private:
  int next_event;
  int cpu_cycles;
  int total_cpu_cycles;

  std::unordered_set<Event*> events;
};

} // namespace nba::core
