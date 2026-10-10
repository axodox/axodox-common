#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingChannel.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "OverlappedOperation.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono_literals;

namespace Axodox::Networking
{
  const size_t _readBufferSize = 64 * 1024;

  named_pipe_messaging_channel::named_pipe_messaging_channel(winrt::file_handle&& pipe, named_pipe_end end) :
    _pipe(move(pipe)),
    _stopEvent(CreateEventW(nullptr, TRUE, FALSE, nullptr))
  {
    winrt::check_bool(bool(_stopEvent));

    ULONG processId = 0, sessionId = 0;
    if (end == named_pipe_end::server)
    {
      winrt::check_bool(GetNamedPipeClientProcessId(_pipe.get(), &processId));
      winrt::check_bool(GetNamedPipeClientSessionId(_pipe.get(), &sessionId));
    }
    else
    {
      winrt::check_bool(GetNamedPipeServerProcessId(_pipe.get(), &processId));
      winrt::check_bool(GetNamedPipeServerSessionId(_pipe.get(), &sessionId));
    }

    _peerProcessId = processId;
    _peerSessionId = sessionId;
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

  uint32_t named_pipe_messaging_channel::peer_process_id() const
  {
    return _peerProcessId;
  }

  uint32_t named_pipe_messaging_channel::peer_session_id() const
  {
    return _peerSessionId;
  }

  void named_pipe_messaging_channel::on_opening()
  {
    auto name = format("peer process {} session {}", _peerProcessId, _peerSessionId);
    _sender = make_unique<background_thread>([this] { send_messages(); }, "* pipe sender - " + name);
    _receiver = make_unique<background_thread>([this] { receive_messages(); }, "* pipe receiver - " + name);
  }

  void named_pipe_messaging_channel::send_messages()
  {
    overlapped_operation operation;
    shared_ptr<message_promise> message;
    while (is_connected())
    {
      if (!_messagesToSend.try_get(message, 200ms)) continue;

      auto& bytes = message->message();
      auto isStarted = WriteFile(_pipe.get(), bytes.data(), DWORD(bytes.size()), nullptr, operation.get()) != FALSE;

      uint32_t written;
      auto error = operation.wait(_pipe.get(), isStarted, _stopEvent.get(), written);
      message->finish(error == ERROR_SUCCESS && written == bytes.size());
      message = nullptr;

      if (error != ERROR_SUCCESS)
      {
        if (error != ERROR_OPERATION_ABORTED) _logger.log(log_severity::information, "Sending failed with error {}, closing the pipe.", error);
        break;
      }
    }

    _messagesToSend.complete();
    _messagesToSend.clear();
    close();
  }

  void named_pipe_messaging_channel::receive_messages()
  {
    overlapped_operation operation;
    vector<uint8_t> buffer(_readBufferSize);
    vector<uint8_t> message;
    while (is_connected())
    {
      auto isStarted = ReadFile(_pipe.get(), buffer.data(), DWORD(buffer.size()), nullptr, operation.get()) != FALSE;

      uint32_t read;
      auto error = operation.wait(_pipe.get(), isStarted, _stopEvent.get(), read);
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

    close();
  }

  void named_pipe_messaging_channel::close()
  {
    SetEvent(_stopEvent.get());
    on_disconnected();
  }
}

#endif
#endif
