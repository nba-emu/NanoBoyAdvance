/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <cassert>
#include <cstdint>
#include <functional>

namespace nba::core {

class SchedulerNew {
public:
  struct Event {
    // TODO: look out for alternatives to std::function.
    std::function<void(int)> callback;
  private:
    friend class SchedulerNew;
    int handle;
    std::uint64_t timestamp;
  };

  SchedulerNew() {
    for (int i = 0; i < kMaxEvents; i++) {
      heap[i] = new Event();
      heap[i]->handle = i;
    }
    Reset();
  }

 ~SchedulerNew() {
    for (int i = 0; i < kMaxEvents; i++)
      delete heap[i];
  }

  void Reset() {
    timestamp_now = 0;
    size = 0;
  }

  auto GetTimestampNow() const -> std::uint64_t {
    return timestamp_now;
  }

  auto GetTimestampTarget() const -> std::uint64_t {
    //assert(size != 0);
    return heap[0]->timestamp;
  }

  auto GetRemainingCycleCount() const -> int {
    return int(GetTimestampTarget() - GetTimestampNow());
  }

  void AddCycles(int cycles) {
    timestamp_now += cycles;
    /*if (timestamp_now >= heap[0]->timestamp && size != 0) {
      Step();
    }*/
  }

  auto Add(std::uint64_t delay, std::function<void(int)> callback) -> Event* {
    int n = size++;
    //assert(n < kMaxEvents);
    auto event = heap[n];
    event->timestamp = GetTimestampNow() + delay;
    event->callback = callback;
    do {
      n = (n - 1) / 2;
    } while (n >= 0 && Heapify(n));
    return event;
  }

  void Cancel(Event* event) {
    Remove(event->handle);
  }

  void Step() {
    auto now = GetTimestampNow();
    while (size > 0 && heap[0]->timestamp <= now) {
      auto event = heap[0];
      Remove(0);
      event->callback(int(GetTimestampNow() - event->timestamp));
    }
  }

private:
  static constexpr int kMaxEvents = 64;

  void Swap(int i, int j) {
    auto tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
    heap[i]->handle = i;
    heap[j]->handle = j;
  }

  bool Heapify(int p) {
    int l = p * 2 + 1;
    int r = p * 2 + 2;
    bool swapped = false;

    if (l < size && heap[l]->timestamp < heap[p]->timestamp) {
      Swap(l, p);
      Heapify(l);
      swapped = true;
    }

    if (r < size && heap[r]->timestamp < heap[p]->timestamp) {
      Swap(r, p);
      Heapify(r);
      swapped = true;
    }

    return swapped;
  }

  void Remove(int n) {
    //assert(size != 0);
    Swap(n, --size);
    Heapify(n);
  }

  std::uint64_t timestamp_now;

  int size;
  Event* heap[kMaxEvents];
};

} // namespace nba::core
