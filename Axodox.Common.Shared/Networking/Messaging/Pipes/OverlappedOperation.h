#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Networking
{
  //One overlapped (asynchronous) operation on a handle, waited for until it completes, or cancelled when a stop event is set first.
  class AXODOX_COMMON_API overlapped_operation
  {
  public:
    overlapped_operation();

    overlapped_operation(const overlapped_operation&) = delete;
    overlapped_operation& operator=(const overlapped_operation&) = delete;

    //The OVERLAPPED structure to start the operation with, e.g. ReadFile(handle, buffer, size, nullptr, operation.get()).
    OVERLAPPED* get();

    //Waits for the operation; isStarted is the result of the call which started it. Returns the Win32 error code
    //(ERROR_SUCCESS if it succeeded, ERROR_OPERATION_ABORTED if it was cancelled) and the transferred byte count.
    uint32_t wait(HANDLE handle, bool isStarted, HANDLE stopEvent, uint32_t& transferred);

  private:
    winrt::handle _event;
    OVERLAPPED _overlapped{};
  };
}

#endif
