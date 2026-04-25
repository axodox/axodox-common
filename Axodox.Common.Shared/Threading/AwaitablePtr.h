#pragma once
#include "Events.h"
#include "Infrastructure/Traits.h"

namespace Axodox::Threading
{
  template <typename T>
  struct awaitable_deleter
  {
    Threading::manual_reset_event* delete_event = nullptr;

    void operator()(T* value)
    {
      if (delete_event) delete_event->set();
      delete value;
    }
  };

  template <typename T>
  using awaitable_ptr = std::unique_ptr<T, awaitable_deleter<T>>;

  template <typename T, typename... TArgs>
  awaitable_ptr<T> make_waitable(Threading::manual_reset_event& deleteEvent, TArgs&&... args)
  {
    return awaitable_ptr<T>(new T(std::forward<TArgs>(args)...), awaitable_deleter<T>{ &deleteEvent });
  }
}