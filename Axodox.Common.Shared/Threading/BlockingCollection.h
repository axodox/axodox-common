#pragma once
#include "common_includes.h"
#include "Events.h"

namespace Axodox::Threading
{
  template<typename T>
  class blocking_collection
  {
  private:
    std::mutex _mutex;
    std::queue<T> _collection;
    Threading::manual_reset_event _event;
    bool _is_complete;

  public:
    blocking_collection() :
      _is_complete(false)
    { }

    void add(T&& item)
    {
      if (_is_complete) throw std::logic_error("Attempted to add new items into a complete blocking collection.");

      std::lock_guard lock(_mutex);
      _collection.push(item);
      _event.set();
    }

    bool try_get(T& item, event_timeout timeout = {})
    {
      for (auto i = 0u; i < 2u; i++)
      {
        {
          std::lock_guard lock(_mutex);
          if (_collection.size() > 0)
          {
            if (_collection.size() == 1) _event.reset();
            item = std::move(_collection.front());
            _collection.pop();
            return true;
          }
          else if (i == 1u || _is_complete)
          {
            return false;
          }
        }

        _event.wait(timeout);
      }

      return false;
    }

    void clear()
    {
      std::lock_guard lock(_mutex);
      while (!_collection.empty())
      {
        _collection.pop();
      }
    }

    void complete()
    {
      _is_complete = true;
      _event.set();
    }

    bool is_complete() const
    {
      return _is_complete;
    }

    ~blocking_collection()
    {
      complete();
    }
  };
}