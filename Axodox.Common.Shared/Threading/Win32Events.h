#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Threading
{
  //A Win32 event, for where a handle is needed: e.g. in an OVERLAPPED structure, or with WaitForMultipleObjects.
  class AXODOX_COMMON_API win32_reset_event
  {
  public:
    void set();
    void reset();

    bool wait(std::chrono::steady_clock::duration timeout = {});

    HANDLE get() const;

  protected:
    win32_reset_event(bool auto_reset, bool signal);

  private:
    winrt::handle _handle;
  };

  struct AXODOX_COMMON_API win32_manual_reset_event : public win32_reset_event
  {
    win32_manual_reset_event(bool signal = false);

    explicit operator bool() const;
  };

  //Has no operator bool, as checking a Win32 auto reset event resets it.
  struct AXODOX_COMMON_API win32_auto_reset_event : public win32_reset_event
  {
    win32_auto_reset_event(bool signal = false);
  };
}

#endif
