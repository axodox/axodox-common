#pragma once
#include "common_includes.h"

namespace Axodox::Threading
{
  class lifetime_source;

  //A movable token which keeps its owning lifetime_source alive for as long as it exists.
  //Acquired via lifetime_source::try_guard(); test with operator bool before relying on it.
  class AXODOX_COMMON_API lifetime_guard
  {
    friend class lifetime_source;

  public:
    lifetime_guard() noexcept;
    ~lifetime_guard();

    lifetime_guard(lifetime_guard&& other) noexcept;
    lifetime_guard& operator=(lifetime_guard&& other) noexcept;

    lifetime_guard(const lifetime_guard&) = delete;
    lifetime_guard& operator=(const lifetime_guard&) = delete;

    //True when this guard is holding its source alive.
    explicit operator bool() const noexcept;

  private:
    explicit lifetime_guard(lifetime_source* owner) noexcept;

    void reset() noexcept;

    lifetime_source* _owner;
  };

  //Held as a member of an object whose methods start detached asynchronous work.
  //Its destructor blocks until every outstanding lifetime_guard is released, so the
  //owning object stays alive while that work runs. Once destruction has begun no new
  //guards can be acquired, letting async methods bail out early.
  class AXODOX_COMMON_API lifetime_source
  {
    friend class lifetime_guard;

  public:
    lifetime_source() noexcept;

    //Begins draining: blocks until all outstanding guards are released.
    ~lifetime_source();

    lifetime_source(const lifetime_source&) = delete;
    lifetime_source& operator=(const lifetime_source&) = delete;

    //Returns a valid guard, or an empty one if the source is already draining.
    [[nodiscard]]
    lifetime_guard try_guard() noexcept;

  private:
    std::mutex _mutex;
    std::condition_variable _drained;
    size_t _guardCount = 0u;
    bool _isDraining = false;
  };
}
