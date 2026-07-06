#include "common_includes.h"
#include "LifetimeGuard.h"

using namespace std;

namespace Axodox::Threading
{
  lifetime_guard::lifetime_guard() noexcept :
    _owner(nullptr)
  { }

  lifetime_guard::lifetime_guard(lifetime_source* owner) noexcept :
    _owner(owner)
  { }

  lifetime_guard::~lifetime_guard()
  {
    reset();
  }

  lifetime_guard::lifetime_guard(lifetime_guard&& other) noexcept :
    _owner(other._owner)
  {
    other._owner = nullptr;
  }

  lifetime_guard& lifetime_guard::operator=(lifetime_guard&& other) noexcept
  {
    if (this != &other)
    {
      reset();
      _owner = other._owner;
      other._owner = nullptr;
    }

    return *this;
  }

  lifetime_guard::operator bool() const noexcept
  {
    return _owner != nullptr;
  }

  void lifetime_guard::reset() noexcept
  {
    if (!_owner) return;

    {
      lock_guard lock(_owner->_mutex);
      if (--_owner->_guardCount == 0u && _owner->_isDraining) _owner->_drained.notify_all();
    }

    _owner = nullptr;
  }

  lifetime_source::lifetime_source() noexcept = default;

  lifetime_source::~lifetime_source()
  {
    unique_lock lock(_mutex);
    _isDraining = true;
    _drained.wait(lock, [this] { return _guardCount == 0u; });
  }

  lifetime_guard lifetime_source::try_guard() noexcept
  {
    lock_guard lock(_mutex);
    if (_isDraining) return {};

    ++_guardCount;
    return lifetime_guard{ this };
  }
}
