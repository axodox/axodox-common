#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"
#include "Win32Events.h"

namespace Axodox::Threading
{
  //One overlapped (asynchronous) operation on a handle opened for overlapped I/O, and not bound to an I/O completion port:
  //e.g. ReadFile, WriteFile, DeviceIoControl or ConnectNamedPipe. It is waited for until it completes, or cancelled when
  //a stop event is set first. One operation at a time; the same object can be reused for the next one.
  class AXODOX_COMMON_API win32_overlapped_operation
  {
  public:
    win32_overlapped_operation() = default;

    win32_overlapped_operation(const win32_overlapped_operation&) = delete;
    win32_overlapped_operation& operator=(const win32_overlapped_operation&) = delete;

    //The OVERLAPPED structure to start the operation with, e.g. ReadFile(handle, buffer, size, nullptr, operation.get()).
    OVERLAPPED* get();

    /// <summary>
    /// Waits until the operation completes. If the stop event is set first, cancels the operation and waits for the
    /// cancellation to finish, so that the OVERLAPPED structure can be reused afterwards.
    /// </summary>
    /// <param name="operationHandle">
    /// The handle the operation was started on. It is used to cancel the operation and to get its result.
    /// </param>
    /// <param name="hasOperationStarted">
    /// The result of the call which started the operation, e.g. <c>ReadFile(...) != FALSE</c>. If it is false, the last
    /// error tells whether the operation is running (ERROR_IO_PENDING, or ERROR_MORE_DATA for a message read): any other
    /// error is returned without waiting. So call this right after starting the operation, before the last error changes.
    /// </param>
    /// <param name="stopEvent">
    /// Cancels the operation when it is set, also if it is set already. An operation which completes before the
    /// cancellation takes effect still reports its own result.
    /// </param>
    /// <param name="transferredBytes">Receives the number of bytes transferred, 0 if the operation could not be started.</param>
    /// <returns>
    /// The Win32 error code: ERROR_SUCCESS if the operation succeeded, ERROR_OPERATION_ABORTED if it was cancelled,
    /// ERROR_MORE_DATA if a message read filled the buffer and more of the message is left, or the error it failed with.
    /// </returns>
    uint32_t wait(HANDLE operationHandle, bool hasOperationStarted, const win32_manual_reset_event& stopEvent, uint32_t& transferredBytes);

  private:
    win32_manual_reset_event _event;
    OVERLAPPED _overlapped{};
  };
}

#endif
