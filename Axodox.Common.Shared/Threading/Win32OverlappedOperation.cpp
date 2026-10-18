#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Win32OverlappedOperation.h"

namespace Axodox::Threading
{
  OVERLAPPED* win32_overlapped_operation::get()
  {
    _event.reset();
    _overlapped = {};
    _overlapped.hEvent = _event.get();
    return &_overlapped;
  }

  uint32_t win32_overlapped_operation::wait(HANDLE operationHandle, bool hasOperationStarted, const win32_manual_reset_event& stopEvent, uint32_t& transferredBytes)
  {
    transferredBytes = 0;

    //Failed immediately, except for an operation which is running, or a message read which completed with more data to come
    if (!hasOperationStarted)
    {
      auto error = GetLastError();
      if (error != ERROR_IO_PENDING && error != ERROR_MORE_DATA) return error;
    }

    //Wait for completion or stop
    HANDLE events[] = { _event.get(), stopEvent.get() };
    if (WaitForMultipleObjects(2, events, FALSE, INFINITE) != WAIT_OBJECT_0)
    {
      CancelIoEx(operationHandle, &_overlapped);
    }

    //The operation must finish (also when cancelled) before the OVERLAPPED structure may be reused
    DWORD byteCount = 0;
    auto error = GetOverlappedResult(operationHandle, &_overlapped, &byteCount, TRUE) ? ERROR_SUCCESS : GetLastError();
    transferredBytes = byteCount;
    return error;
  }
}

#endif
