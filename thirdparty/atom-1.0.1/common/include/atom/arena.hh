// SPDX-FileCopyrightText: Copyright 2026 Mireille Meyer
// SPDX-License-Identifier: 0BSD

#pragma once

#include <atom/integer.hh>
#include <atom/panic.hh>
#include <atom/non_copyable.hh>
#include <algorithm>

#if defined(WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#elif defined(__APPLE__)
  #include <sys/mman.h>
#endif

//#define ATOM_ARENA_USE_MALLOC

namespace atom {

class Arena : NonCopyable {
  public:
    // Implementation of allocator named requirements for Arena.
    // TODO(fleroviux): should Allocator hold ownership over the Arena?
    template<typename T>
    struct Allocator {
      using value_type = T;

      explicit Allocator(Arena& arena) : m_arena{arena} {}

      [[nodiscard]] T* allocate(size_t n) {
        return static_cast<T*>(m_arena.Allocate(sizeof(T) * n));
      }

      void deallocate(T* p, size_t n) {}

      [[nodiscard]] bool operator==(const Allocator& _) {
        // "`true` only if the storage allocated by the allocator a1 can be deallocated through a2."
        // This is always the case as deallocation is a no-op.
        return true;
      }

      private:
        Arena& m_arena;
    };

    explicit Arena(size_t capacity) {
#if defined(WIN32) && !defined(ATOM_ARENA_USE_MALLOC)
      m_base_address = (u8*)VirtualAlloc(nullptr, capacity, MEM_COMMIT, PAGE_READWRITE);
#elif defined(__APPLE__) && !defined(ATOM_ARENA_USE_MALLOC)
      m_base_address = (u8*)mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
      m_base_address = (u8*)std::malloc(capacity);
#endif

      if(m_base_address == nullptr) {
        ATOM_PANIC("atom: out of memory");
      }
      m_maximum_address = m_base_address + capacity;
      Reset();
    }

    Arena(Arena&& arena) noexcept {
      *this = std::move(arena);
    }

   ~Arena() {
      if(m_base_address) {
        const size_t capacity = m_maximum_address - m_base_address;

#if defined(WIN32) && !defined(ATOM_ARENA_USE_MALLOC)
        VirtualFree(m_base_address, capacity, MEM_RELEASE);
#elif defined(__APPLE__) && !defined(ATOM_ARENA_USE_MALLOC)
        munmap(m_base_address, capacity);
#else
        (void)capacity;
        std::free(m_base_address);
#endif
      }
    }

    Arena& operator=(Arena&& arena) noexcept {
      std::swap(m_base_address, arena.m_base_address);
      std::swap(m_current_address, arena.m_current_address);
      std::swap(m_maximum_address, arena.m_maximum_address);
      return *this;
    }

    void Reset() {
      m_current_address = m_base_address;
    }

    void* Allocate(size_t number_of_bytes) {
      u8* address = m_current_address;
      m_current_address += number_of_bytes;
      if(m_current_address <= m_maximum_address) {
        return address;
      }
      return nullptr;
    }

  private:
    u8* m_base_address{};
    u8* m_current_address{};
    u8* m_maximum_address{};
};

}  // namespace atom
