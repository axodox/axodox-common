#include "common_includes.h"
#include "Events.h"

using namespace std;

namespace Axodox::Threading
{
  reset_event::reset_event(bool auto_reset, bool signal) :
    _auto_reset(auto_reset),
    _signal(signal)
  { }

  void reset_event::set()
  {
    {
      lock_guard<mutex> lock(_mutex);
      _signal = true;
    }

    _condition.notify_all();
  }

  void reset_event::reset()
  {
    lock_guard<mutex> lock(_mutex);
    _signal = false;
  }

  bool reset_event::wait(std::chrono::steady_clock::duration timeout)
  {
    unique_lock<mutex> lock(_mutex);

    bool result;
    if (timeout != chrono::steady_clock::duration{})
    {
      result = _condition.wait_for(lock, timeout, [&] { return _signal; });
    }
    else
    {
      _condition.wait(lock, [&] { return _signal; });
      result = true;
    }

    if (_auto_reset) _signal = false;
    return result;
  }

  reset_event::operator bool() const
  {
    return _signal;
  }

  manual_reset_event::manual_reset_event(bool signal) :
    reset_event(false, signal)
  { }

  auto_reset_event::auto_reset_event(bool signal) :
    reset_event(true, signal)
  { }
}