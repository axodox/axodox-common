#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  class recursion_lock;

  class AXODOX_COMMON_API recursion_counter
  {
    friend class recursion_lock;

  public:
    [[nodiscard]]
    recursion_lock lock();

    bool is_locked() const;

  private:
    size_t _lockCounter = 0u;
  };

  class AXODOX_COMMON_API recursion_lock
  {
  public:
    recursion_lock();
    explicit recursion_lock(recursion_counter* owner);
    ~recursion_lock();

    recursion_lock(recursion_lock&& other);
    recursion_lock& operator=(recursion_lock&& other);

    recursion_lock(const recursion_lock&) = delete;
    recursion_lock& operator=(const recursion_lock&) = delete;

  private:
    recursion_counter* _owner;
  };
}