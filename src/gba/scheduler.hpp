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