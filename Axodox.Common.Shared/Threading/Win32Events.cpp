#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Win32Events.h"

using namespace std;
using namespace winrt;

namespace Axodox::Threading
{
  win32_reset_event::win32_reset_event(bool auto_reset, bool signal) :
    _handle(CreateEventW(nullptr, !auto_reset, signal, nullptr))
  {
    check_bool(bool(_handle));
  }

  void win32_reset_event::set()
  {
    SetEvent(_handle.get());
  }

  void win32_reset_event::reset()
  {
    ResetEvent(_handle.get());
  }

  bool win32_reset_event::wait(std::chrono::steady_clock::duration timeout)
  {
    auto milliseconds = INFINITE;
    if (timeout != chrono::steady_clock::duration{})
    {
      //Rounded up, so that a short timeout does not become a poll; INFINITE itself is not a valid timeout here
      auto count = chrono::ceil<chrono::milliseconds>(timeout).count();
      milliseconds = DWORD(clamp<decltype(count)>(count, 0, INFINITE - 1));
    }

    return WaitForSingleObject(_handle.get(), milliseconds) == WAIT_OBJECT_0;
  }

  HANDLE win32_reset_event::get() const
  {
    return _handle.get();
  }

  win32_manual_reset_event::win32_manual_reset_event(bool signal) :
    win32_reset_event(false, signal)
  { }

  win32_manual_reset_event::operator bool() const
  {
    return WaitForSingleObject(get(), 0) == WAIT_OBJECT_0;
  }

  win32_auto_reset_event::win32_auto_reset_event(bool signal) :
    win32_reset_event(true, signal)
  { }
}

#endif
