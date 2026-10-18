#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingChannel.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "Infrastructure/Exceptions.h"
#include "Infrastructure/Win32.h"
#include "Threading/Win32OverlappedOperation.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace winrt;
using namespace std::chrono_literals;

namespace Axodox::Networking
{
  const size_t _readBufferSize = 64 * 1024;

  named_pipe_messaging_channel::named_pipe_messaging_channel(file_handle&& pipe, named_pipe_end end) :
    _pipe(move(pipe))
  {
    ULONG processId = 0, sessionId = 0;
    if (end == named_pipe_end::server)
    {
      check_bool(GetNamedPipeClientProcessId(_pipe.get(), &processId));
      check_bool(GetNamedPipeClientSessionId(_pipe.get(), &sessionId));
    }
    else
    {
      check_bool(GetNamedPipeServerProcessId(_pipe.get(), &processId));
      check_bool(GetNamedPipeServerSessionId(_pipe.get(), &sessionId));
    }

    _remotePeer = { .process_id = processId, .session_id = sessionId };
  }

  named_pipe_messaging_channel::~named_pipe_messaging_channel()
  {
    close();
    _messagesToSend.complete();
    _receiver.reset();
    _sender.reset();
  }

  message_task named_pipe_messaging_channel::send_message(vector<uint8_t>&& message)
  {
    auto messagePromise = make_shared<message_promise>(move(message));
    message_task task{ messagePromise };
    _messagesToSend.add(move(messagePromise));
    return task;
  }

  named_pipe_peer_description named_pipe_messaging_channel::remote_peer() const
  {
    return _remotePeer;
  }

  std::wstring named_pipe_messaging_channel::pipe_path(std::string_view name)
  {
    return L"\\\\.\\pipe\\" + to_wstring(name);
  }

  void named_pipe_messaging_channel::on_opening()
  {
    auto name = format("peer process {} session {}", _remotePeer.process_id, _remotePeer.session_id);
    _sender = make_unique<background_thread>([this] { send_messages(); }, "* pipe sender - " + name);
    _receiver = make_unique<background_thread>([this] { receive_messages(); }, "* pipe receiver - " + name);
  }

  void named_pipe_messaging_channel::send_messages()
  {
    try
    {
      win32_overlapped_operation operation;
      shared_ptr<message_promise> message;
      while (is_connected())
      {
        if (!_messagesToSend.try_get(message, 200ms)) continue;

        auto& bytes = message->message();
        auto isStarted = WriteFile(_pipe.get(), bytes.data(), DWORD(bytes.size()), nullptr, operation.get()) != FALSE;

        uint32_t written;
        auto error = operation.wait(_pipe.get(), isStarted, _stopEvent, written);
        message->finish(error == ERROR_SUCCESS && written == bytes.size());
        message = nullptr;

        if (error != ERROR_SUCCESS)
        {
          if (error != ERROR_OPERATION_ABORTED) _logger.log(log_severity::information, "Sending failed with error {}, closing the pipe.", error);
          break;
        }
      }
    }
    catch (...)
    {
      _logger.log(log_severity::error, "Failed to send messages, closing the pipe. Reason: {}", get_exception_message());
    }

    _messagesToSend.complete();
    _messagesToSend.clear();
    close();
  }

  void named_pipe_messaging_channel::receive_messages()
  {
    //A message_received handler which throws closes the pipe, as with TCP
    try
    {
      win32_overlapped_operation operation;
      vector<uint8_t> buffer(_readBufferSize);
      vector<uint8_t> message;
      while (is_connected())
      {
        auto isStarted = ReadFile(_pipe.get(), buffer.data(), DWORD(buffer.size()), nullptr, operation.get()) != FALSE;

        uint32_t read;
        auto error = operation.wait(_pipe.get(), isStarted, _stopEvent, read);
        if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
        {
          if (error != ERROR_OPERATION_ABORTED) _logger.log(log_severity::information, "The pipe was closed (error {}).", error);
          break;
        }

        //A message larger than the buffer arrives in parts, all but the last one with ERROR_MORE_DATA
        message.insert(message.end(), buffer.begin(), buffer.begin() + read);
        if (error == ERROR_MORE_DATA) continue;

        if (!message.empty()) on_received(message);
        message.clear();
      }
    }
    catch (...)
    {
      _logger.log(log_severity::error, "Failed to receive messages, closing the pipe. Reason: {}", get_exception_message());
    }

    close();
  }

  void named_pipe_messaging_channel::close()
  {
    _stopEvent.set();
    on_disconnected();
  }
}

#endif
#endif
