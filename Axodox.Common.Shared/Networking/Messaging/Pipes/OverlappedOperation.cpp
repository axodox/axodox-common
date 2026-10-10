#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "OverlappedOperation.h"

namespace Axodox::Networking
{
  overlapped_operation::overlapped_operation() :
    _event(CreateEventW(nullptr, TRUE, FALSE, nullptr))
  {
    winrt::check_bool(bool(_event));
  }

  OVERLAPPED* overlapped_operation::get()
  {
    ResetEvent(_event.get());
    _overlapped = {};
    _overlapped.hEvent = _event.get();
    return &_overlapped;
  }

  uint32_t overlapped_operation::wait(HANDLE handle, bool isStarted, HANDLE stopEvent, uint32_t& transferred)
  {
    transferred = 0;

    //Failed immediately, except for an operation which is running, or a message read which completed with more data to come
    if (!isStarted)
    {
      auto error = GetLastError();
      if (error != ERROR_IO_PENDING && error != ERROR_MORE_DATA) return error;
    }

    //Wait for completion or stop
    HANDLE events[] = { _event.get(), stopEvent };
    if (WaitForMultipleObjects(2, events, FALSE, INFINITE) != WAIT_OBJECT_0)
    {
      CancelIoEx(handle, &_overlapped);
    }

    //The operation must finish (also when cancelled) before the OVERLAPPED structure may be reused
    DWORD byteCount = 0;
    auto error = GetOverlappedResult(handle, &_overlapped, &byteCount, TRUE) ? ERROR_SUCCESS : GetLastError();
    transferred = byteCount;
    return error;
  }
}

#endif
