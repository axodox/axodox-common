#pragma once
#include "common_includes.h"

namespace Axodox::Threading
{
  template<typename T>
  struct default_unlock
  {
    void operator()(T*) { }
  };

  template<typename TValue, typename TUnlock = default_unlock<TValue>>
  class locked_ptr
  {
  public:
    using mutex_t = std::shared_mutex;
    using lock_t = std::shared_lock<mutex_t>;
    using value_t = TValue;
    using unlock_t = TUnlock;

    locked_ptr() :
      _value(nullptr)
    { }

    locked_ptr(mutex_t& mutex, value_t* value) :
      _lock(mutex),
      _value(value)
    { }

    locked_ptr(lock_t&& lock, value_t* value) :
      _lock(std::move(lock)),
      _value(value)
    { }

    locked_ptr(locked_ptr&& other)
    { 
      *this = std::move(other);
    }

    locked_ptr& operator=(locked_ptr&& other)
    {
      unlock_t{}(_value);
      _value = std::exchange(other._value, nullptr);
      _lock = std::exchange(other._lock, {});
      return *this;
    }

    ~locked_ptr()
    {
      reset();
    }

    void reset()
    {
      unlock_t{}(_value);
      _value = nullptr;
      _lock = {};
    }

    value_t* get() const
    {
      return _value;
    }

    value_t& operator*() const
    {
      return *_value;
    }

    value_t* operator->() const
    {
      return _value;
    }

    explicit operator bool() const
    {
      return _lock;
    }

  private:
    lock_t _lock;
    value_t* _value;
  };
}