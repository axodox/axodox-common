#include "common_includes.h"
#include "RecursionLock.h"

using namespace std;

namespace Axodox::Threading
{
  recursion_lock recursion_counter::lock()
  {
    return recursion_lock{ this };
  }

  bool recursion_counter::is_locked() const
  {
    return _lockCounter > 0u;
  }

  recursion_lock::recursion_lock() :
    _owner(nullptr)
  { }

  recursion_lock::recursion_lock(recursion_counter* owner) :
    _owner(owner)
  {
    if (_owner) _owner->_lockCounter++;
  }

  recursion_lock::~recursion_lock()
  {
    if (_owner) _owner->_lockCounter--;
  }

  recursion_lock::recursion_lock(recursion_lock&& other)
  {
    swap(_owner, other._owner);
  }

  recursion_lock& recursion_lock::operator=(recursion_lock&& other)
  {
    swap(_owner, other._owner);
    return *this;
  }
}