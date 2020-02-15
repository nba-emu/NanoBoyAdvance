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

namespace GameBoyAdvance {

using cycle_t = int;

struct Event {
  cycle_t countdown = 0;
  
  std::function<void()> tick;
};

class Scheduler {
public:
  void Reset() {
    events.clear();
  }
  
  void Add(Event& event) {
    events.insert(&event);
  }

  void Remove(Event& event) {
    events.erase(&event);
  }
  
  auto Schedule(cycle_t elapsed) -> cycle_t {
    cycle_t ticks_to_event = std::numeric_limits<cycle_t>::max();
    
    for (auto it = events.begin(); it != events.end();) {
      auto event = *it;
      ++it;
      event->countdown -= elapsed;
      if (event->countdown <= 0) {
        event->tick();
      }
      if (event->countdown < ticks_to_event) {
        ticks_to_event = event->countdown;
      }
    }
    
    return ticks_to_event;
  }

private:
  std::unordered_set<Event*> events;
};

}